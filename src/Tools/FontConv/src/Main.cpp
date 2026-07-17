#include <IO/Xml.h>
#include <IO/FileStream.h>
#include <IO/StreamWriter.h>
#include <IO/Path/Path.h>
#include <IO/Path/File.h>
#include <vector>

using namespace Starshine;
using namespace Starshine::Xml;
using namespace Starshine::IO;

static constexpr char CurrentRevision{ 0 };
static constexpr std::array<char, 4> FontSignature{ 'S', 'F', 'T', CurrentRevision };

struct FontHeader
{
	i32 GlyphSize{};
	i32 LineHeight{};
	i32 Baseline{};
};

struct FontGlyph
{
	u16 X{};
	u16 Y{};
	u16 Width{};
	u16 Height{};

	i16 XOffset{};
	i16 YOffset{};

	i16 XAdvance{};
};

struct GlyphRangeMapping
{
	i32 CharCode{};
	i32 FirstGlyph{};
	size_t GlyphCount{};
};

struct GlyphKerning
{
	i32 SecondCharCode{};
	i32 Amount{};
};

struct GlyphKerningList
{
	i32 CharCode{};
	std::vector<GlyphKerning> Kernings;
};

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		return 1;
	}

	// --------------

	std::string_view inputFilePath = argv[1];

	Xml::Document fileDoc = {};
	Xml::ParseFromFile(fileDoc, inputFilePath);

	const Xml::Element* fontElement = fileDoc.FirstChildElement("font");
	const Xml::Element* fontInfo = fontElement->FirstChildElement("info");
	const Xml::Element* fontCommon = fontElement->FirstChildElement("common");

	FontHeader header{};
	fontInfo->QueryAttribute("size", &header.GlyphSize);
	fontCommon->QueryAttribute("lineHeight", &header.LineHeight);
	fontCommon->QueryAttribute("base", &header.Baseline);

	const Xml::Element* fontChars = fontElement->FirstChildElement("chars");
	size_t charCount = fontChars->Unsigned64Attribute("count");

	std::vector<FontGlyph> glyphs;
	glyphs.reserve(charCount);

	std::vector<GlyphRangeMapping> glyphRanges;
	GlyphRangeMapping currentGlyphRange{ -1, 0, 0 };

	const Xml::Element* charElement = fontChars->FirstChildElement("char");
	for (size_t i = 0; i < charCount; i++)
	{
		i32 id = charElement->IntAttribute("id");

		u16 x = charElement->IntAttribute("x");
		u16 y = charElement->IntAttribute("y");
		u16 width = charElement->IntAttribute("width");
		u16 height = charElement->IntAttribute("height");
		
		i16 xOffset = charElement->IntAttribute("xoffset");
		i16 yOffset = charElement->IntAttribute("yoffset");
		i16 xAdvance = charElement->IntAttribute("xadvance");

		glyphs.emplace_back(FontGlyph {x, y, width, height, xOffset, yOffset, xAdvance});

		if (id - (currentGlyphRange.CharCode + currentGlyphRange.GlyphCount - 1) > 1 && (id != currentGlyphRange.CharCode))
		{
			glyphRanges.push_back(currentGlyphRange);
			currentGlyphRange = { id, static_cast<i32>(i), 0 };
		}
		currentGlyphRange.GlyphCount++;

		charElement = charElement->NextSiblingElement();
	}

	const Xml::Element* fontKernings = fontElement->FirstChildElement("kernings");
	std::vector<GlyphKerningList> glyphKernings;

	if (fontKernings != nullptr)
	{
		size_t kerningCount = fontKernings->Unsigned64Attribute("count");
		const Xml::Element* kerningElement = fontKernings->FirstChildElement("kerning");

		for (size_t i = 0; i < kerningCount; i++)
		{
			i32 first = kerningElement->IntAttribute("first");
			i32 second = kerningElement->IntAttribute("second");
			i32 amount = kerningElement->IntAttribute("amount");

			for (auto& kern : glyphKernings)
			{
				if (kern.CharCode == first)
				{
					kern.Kernings.emplace_back(GlyphKerning{ second, amount });
					goto kernFound;
				}
			}

			goto kernNotFound;

		kernFound:
			kerningElement = kerningElement->NextSiblingElement();
			continue;

		kernNotFound:
			auto& newKern = glyphKernings.emplace_back();
			newKern.CharCode = first;
			newKern.Kernings.emplace_back(GlyphKerning{ second, amount });

			kerningElement = kerningElement->NextSiblingElement();
		}
	}

	fileDoc.Clear();

	// --------------

	std::string outputFilePath = Path::ChangeExtension(inputFilePath, ".dat");

	FileStream outputFile = File::CreateWrite(outputFilePath);
	StreamWriter writer = StreamWriter(outputFile);
	writer.SetPointerSize(PointerSize::Size32Bit);
	
	writer.WriteBuffer(FontSignature.data(), FontSignature.size());
	writer.WriteBuffer(&header, sizeof(FontHeader));

	writer.WriteSize(glyphs.size());
	writer.WriteFunctionPointer([&](StreamWriter& writer)
		{
			for (auto& glyph : glyphs)
			{
				writer.WriteU16(glyph.X);
				writer.WriteU16(glyph.Y);
				writer.WriteU16(glyph.Width);
				writer.WriteU16(glyph.Height);

				writer.WriteI16(glyph.XOffset);
				writer.WriteI16(glyph.YOffset);
				writer.WriteI16(glyph.XAdvance);

				writer.WriteAlignedPadding(16);
			}
		});

	writer.WriteSize(glyphRanges.size());
	writer.WriteFunctionPointer([&](StreamWriter& writer)
		{
			for (auto& glyphRange : glyphRanges)
			{
				writer.WriteI32(glyphRange.CharCode);
				writer.WriteI32(glyphRange.FirstGlyph);
				writer.WriteSize(glyphRange.GlyphCount);

				writer.WriteAlignedPadding(16);
			}
		});

	writer.WriteSize(glyphKernings.size());
	if (glyphKernings.size() > 0)
	{
		writer.WriteFunctionPointer([&](StreamWriter& writer)
			{
				for (auto& glyphKernList : glyphKernings)
				{
					writer.WriteI32(glyphKernList.CharCode);
					writer.WriteSize(glyphKernList.Kernings.size());

					for (auto& kern : glyphKernList.Kernings)
					{
						writer.WriteI32(kern.SecondCharCode);
						writer.WriteI32(kern.Amount);
					}

					writer.WriteAlignedPadding(16);
				}
			});
	}
	else
	{
		writer.WritePointer(0);
	}

	writer.WriteAlignedPadding(16);
	writer.FlushFunctionArray();
	outputFile.Close();

	return 0;
}
