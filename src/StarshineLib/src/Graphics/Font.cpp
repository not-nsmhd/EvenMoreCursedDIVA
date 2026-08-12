#include "Font.h"
#include "IO/Xml.h"
#include "IO/Path/File.h"
#include "IO/Path/Path.h"
#include "Common/Logging/Logging.h"
#include "Misc/ImageHelper.h"

namespace Starshine::Graphics
{
	using namespace IO;

	namespace FileFormatDetail
	{
		static constexpr u8 CurrentRevision = 1;
		static constexpr std::array<char, 4> FileSignature = { 'S', 'F', 'F', CurrentRevision };
	}

	bool Font::ReadBMFont(std::string_view basePath, const char* xmlData, size_t xmlSize, TextureFormat texFormat)
	{
		if (xmlData == nullptr || xmlSize == 0) { return false; }

		Xml::Document document = Xml::Document();
		document.Parse(xmlData, xmlSize);

		if (document.Error())
		{
			LogMessage("Failed to parse BMFont file. Error: %s", document.ErrorStr());
			document.Clear();
			return false;
		}

		const Xml::Element* fontElement = document.FirstChildElement("font");
		const Xml::Element* fontInfo = fontElement->FirstChildElement("info");
		const Xml::Element* fontCommon = fontElement->FirstChildElement("common");

		fontInfo->QueryAttribute("size", &Size);
		fontCommon->QueryAttribute("lineHeight", &LineHeight);
		fontCommon->QueryAttribute("base", &Baseline);

		const Xml::Element* pagesElement = fontElement->FirstChildElement("pages");
		const Xml::Element* textureElement = pagesElement->FirstChildElement("page");
		const Xml::Attribute* texturePathAttrib = textureElement->FindAttribute("file");
		
		std::string texturePath = std::string(basePath);
		texturePath.append("/");
		texturePath.append(texturePathAttrib->Value());

		if (!Misc::ImageHelper::ReadImageFile(texturePath, TextureImage, texFormat))
		{
			LogMessage("Failed to load font texture");
			document.Clear();
			return false;
		}

		switch (texFormat)
		{
		case TextureFormat::R8:
			type = FontType::SingleChannel;
			break;
		case TextureFormat::RG8:
			type = FontType::FillOutlineRG;
			break;
		}

		const Xml::Element* charsElement = fontElement->FirstChildElement("chars");

		size_t charCount = 0;
		const Xml::Attribute* charCountAttrib = charsElement->FindAttribute("count");
		charCountAttrib->QueryUnsigned64Value(&charCount);

		Glyphs.reserve(static_cast<size_t>(charCount));
		GlyphRangeMapping currentGlyphRange{ -1, 0, 0 };

		const Xml::Element* glyphElement = charsElement->FirstChildElement("char");
		for (size_t i = 0; i < charCount; i++)
		{
			if (glyphElement == nullptr)
			{
				GlyphCharMappings.push_back(currentGlyphRange);
				break;
			}

			i32 id = glyphElement->IntAttribute("id");

			u16 x = glyphElement->IntAttribute("x");
			u16 y = glyphElement->IntAttribute("y");
			u16 width = glyphElement->IntAttribute("width");
			u16 height = glyphElement->IntAttribute("height");

			i8 xOffset = glyphElement->IntAttribute("xoffset");
			i8 yOffset = glyphElement->IntAttribute("yoffset");
			i16 xAdvance = glyphElement->IntAttribute("xadvance");

			Glyphs.emplace_back(FontGlyph{ x, y, width, height, xOffset, yOffset, xAdvance });

			if (id - (currentGlyphRange.CharCode + currentGlyphRange.GlyphCount - 1) > 1 && (id != currentGlyphRange.CharCode))
			{
				GlyphCharMappings.push_back(currentGlyphRange);
				currentGlyphRange = { id, static_cast<i32>(i), 0 };
			}
			currentGlyphRange.GlyphCount++;

			glyphElement = glyphElement->NextSiblingElement();
		}

		const Xml::Element* fontKernings = fontElement->FirstChildElement("kernings");

		if (fontKernings != nullptr)
		{
			size_t kerningCount = fontKernings->Unsigned64Attribute("count");
			const Xml::Element* kerningElement = fontKernings->FirstChildElement("kerning");

			for (size_t i = 0; i < kerningCount; i++)
			{
				i32 first = kerningElement->IntAttribute("first");
				i32 second = kerningElement->IntAttribute("second");
				i32 amount = kerningElement->IntAttribute("amount");

				for (auto& kern : GlyphKernings)
				{
					if (kern.CharCode == first)
					{
						kern.KerningList.emplace_back(GlyphKerning{ second, amount });
						goto kernFound;
					}
				}

				goto kernNotFound;

			kernFound:
				kerningElement = kerningElement->NextSiblingElement();
				continue;

			kernNotFound:
				auto& newKern = GlyphKernings.emplace_back();
				newKern.CharCode = first;
				newKern.KerningList.emplace_back(GlyphKerning{ second, amount });

				kerningElement = kerningElement->NextSiblingElement();
			}

			resolveKerningReferences();
		}

		return true;
	}

	bool Font::ReadBMFont(const std::string_view filePath, TextureFormat texFormat)
	{
		std::string_view basePath = Path::GetDirectoryPath(filePath);

		std::unique_ptr<u8[]> xmlData;
		size_t xmlSize = File::ReadAllBytes(filePath, xmlData);

		if (xmlData == nullptr || xmlSize == 0)
			return false;

		bool result = ReadBMFont(basePath, reinterpret_cast<const char*>(xmlData.get()), xmlSize, texFormat);
		return result;
	}

	bool Font::ReadBinary(IO::StreamReader& reader)
	{
		char signature[4]{};
		reader.ReadBuffer(signature, sizeof(signature));
		if (SDL_memcmp(signature, FileFormatDetail::FileSignature.data(), FileFormatDetail::FileSignature.size()) != 0) { return false; }

		type = static_cast<FontType>(reader.ReadI32());

		Size = reader.ReadI32();
		LineHeight = reader.ReadI32();
		Baseline = reader.ReadI32();

		const size_t glyphCount = reader.ReadSize();
		const size_t glyphArrayOffset = reader.ReadPointer();

		const size_t charMappingCount = reader.ReadSize();
		const size_t charMappingOffset = reader.ReadPointer();

		const size_t kerningCount = reader.ReadSize();
		const size_t kerningOffset = reader.ReadPointer();

		const size_t textureCount = reader.ReadSize();
		const size_t textureOffset = reader.ReadSize();

		Glyphs.reserve(glyphCount);
		reader.ReadAt(glyphArrayOffset, [&](IO::StreamReader& reader)
			{
				for (size_t i = 0; i < glyphCount; i++)
				{
					FontGlyph glyph{};

					glyph.X = reader.ReadU16();
					glyph.Y = reader.ReadU16();
					glyph.Width = reader.ReadU16();
					glyph.Height = reader.ReadU16();

					glyph.XOffset = reader.ReadI8();
					glyph.YOffset = reader.ReadI8();
					glyph.XAdvance = reader.ReadI16();

					Glyphs.push_back(glyph);
					reader.SkipUntilAligned(16);
				}
			});

		GlyphCharMappings.reserve(charMappingCount);
		reader.ReadAt(charMappingOffset, [&](IO::StreamReader& reader)
			{
				for (size_t i = 0; i < charMappingCount; i++)
				{
					GlyphRangeMapping mapping{};

					mapping.CharCode = reader.ReadI32();
					mapping.FirstGlyph = reader.ReadI32();
					mapping.GlyphCount = reader.ReadSize();

					GlyphCharMappings.push_back(mapping);
					reader.SkipUntilAligned(16);
				}
			});

		if (kerningCount > 0 && kerningOffset > 0)
		{
			GlyphKernings.reserve(kerningCount);
			reader.ReadAt(kerningOffset, [&](IO::StreamReader& reader)
				{
					for (size_t i = 0; i < kerningCount; i++)
					{
						GlyphKerningList kernList{};

						kernList.CharCode = reader.ReadI32();
						size_t kernListSize = reader.ReadSize();

						kernList.KerningList.reserve(kernListSize);
						for (size_t j = 0; j < kernListSize; j++)
						{
							GlyphKerning kern{};

							kern.SecondCharCode = reader.ReadI32();
							kern.Amount = reader.ReadI32();

							kernList.KerningList.push_back(kern);
						}

						GlyphKernings.push_back(kernList);
						reader.SkipUntilAligned(16);
					}
				});

			resolveKerningReferences();
		}

		TextureImage = std::make_unique<Texture>();
		reader.ReadAt(textureOffset, [&](StreamReader& reader) { TextureImage->ReadBinary(reader); });

		return true;
	}

	void Font::WriteBinary(IO::StreamWriter& writer)
	{
		writer.WriteBuffer(FileFormatDetail::FileSignature.data(), FileFormatDetail::FileSignature.size());
		writer.WriteI32(static_cast<i32>(type));

		writer.WriteI32(Size);
		writer.WriteI32(LineHeight);
		writer.WriteI32(Baseline);

		writer.WriteSize(Glyphs.size());
		writer.WriteFunctionPointer([&](StreamWriter& writer)
			{
				for (auto& glyph : Glyphs)
				{
					writer.WriteU16(glyph.X);
					writer.WriteU16(glyph.Y);
					writer.WriteU16(glyph.Width);
					writer.WriteU16(glyph.Height);

					writer.WriteI8(glyph.XOffset);
					writer.WriteI8(glyph.YOffset);
					writer.WriteI16(glyph.XAdvance);

					writer.WriteAlignedPadding(16);
				}
			});

		writer.WriteSize(GlyphCharMappings.size());
		writer.WriteFunctionPointer([&](StreamWriter& writer)
			{
				for (auto& glyphRange : GlyphCharMappings)
				{
					writer.WriteI32(glyphRange.CharCode);
					writer.WriteI32(glyphRange.FirstGlyph);
					writer.WriteSize(glyphRange.GlyphCount);

					writer.WriteAlignedPadding(16);
				}
			});

		writer.WriteSize(GlyphKernings.size());
		if (GlyphKernings.size() > 0)
		{
			writer.WriteFunctionPointer([&](StreamWriter& writer)
				{
					for (auto& glyphKernList : GlyphKernings)
					{
						writer.WriteI32(glyphKernList.CharCode);
						writer.WriteSize(glyphKernList.KerningList.size());

						for (auto& kern : glyphKernList.KerningList)
						{
							writer.WriteI32(kern.SecondCharCode);
							writer.WriteI32(kern.Amount);
						}

						writer.WriteAlignedPadding(16);
					}
				});
		}
		else
			writer.WritePointer(0);

		writer.WriteSize(1); // NOTE/TODO: Texture count
		writer.WriteFunctionPointer([&](StreamWriter& writer) { TextureImage->WriteBinary(writer); });

		writer.WriteAlignedPadding(16);
		writer.FlushFunctionArray();
	}

	void Font::Destroy()
	{
		TextureImage = nullptr;
		Glyphs.clear();
	}

	const FontGlyph* Font::GetGlyph(i32 code) const
	{
		if (GlyphCharMappings.size() == 0) { return nullptr; }
		FontGlyph* result{};

		for (const auto& mapping : GlyphCharMappings)
		{
			if (code >= mapping.CharCode && code < (mapping.CharCode + mapping.GlyphCount))
				return &Glyphs[mapping.FirstGlyph + (code - mapping.CharCode)];
		}

		return &Glyphs[0];
	}

	FontType Font::GetType() const
	{
		return type;
	}

	FontGlyph* Font::getGlyph(i32 code)
	{
		if (GlyphCharMappings.size() == 0) { return nullptr; }
		FontGlyph* result{};

		for (const auto& mapping : GlyphCharMappings)
		{
			if (code >= mapping.CharCode && code < (mapping.CharCode + mapping.GlyphCount))
				return &Glyphs[mapping.FirstGlyph + (code - mapping.CharCode)];
		}

		return &Glyphs[0];
	}

	void Font::resolveKerningReferences()
	{
		if (GlyphKernings.size() > 0)
		{
			for (auto& kernList : GlyphKernings)
			{
				FontGlyph* glyph = getGlyph(kernList.CharCode);
				glyph->KerningList = &kernList;
			}
		}
	}
}
