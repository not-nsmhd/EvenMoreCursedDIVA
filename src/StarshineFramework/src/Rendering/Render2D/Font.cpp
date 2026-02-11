#include "Font.h"
#include "IO/Xml.h"
#include "IO/Path/File.h"
#include "IO/Path/Path.h"
#include "Common/Logging/Logging.h"
#include "Misc/ImageHelper.h"

namespace Starshine::Rendering::Render2D
{
	using namespace IO;

	Font::~Font()
	{
		Glyphs.clear();
		//Texture = nullptr;
	}

	const FontGlyph* Font::GetGlyph(i32 code) const
	{
		for (auto& glyph : Glyphs)
		{
			if (glyph.CharacterCode == code)
			{
				return &glyph;
			}
		}

		return &Glyphs.at(0);
	}

	bool Font::ReadBMFont(std::string_view basePath, const std::unique_ptr<u8[]>& xmlData, size_t xmlSize)
	{
		if (xmlData == nullptr || xmlSize == 0) { return false; }

		Xml::Document document = Xml::Document();
		document.Parse(reinterpret_cast<const char*>(xmlData.get()), xmlSize);

		if (document.Error())
		{
			LogMessage("Failed to parse BMFont file. Error: %s", document.ErrorStr());
			document.Clear();
			return false;
		}

		Xml::Element* rootElement = document.FirstChildElement("font");

		Xml::Element* commonElement = rootElement->FirstChildElement("common");
		const Xml::Attribute* lineHeightAttrib = commonElement->FindAttribute("lineHeight");

		lineHeightAttrib->QueryIntValue(&LineHeight);

		Xml::Element* pagesElement = rootElement->FirstChildElement("pages");
		const Xml::Element* textureElement = pagesElement->FirstChildElement("page");
		const Xml::Attribute* texturePathAttrib = textureElement->FindAttribute("file");
		
		std::string texturePath = std::string(basePath);
		texturePath.append("/");
		texturePath.append(texturePathAttrib->Value());

		ivec2 texSize{};
		i32 channels{};
		std::unique_ptr<u8[]> texData{};
		if (!Misc::ImageHelper::ReadImageFile(texturePath, texSize, channels, texData))
		{
			LogMessage("Failed to load font texture");
			document.Clear();
			return false;
		}

		Texture = Rendering::GetDevice()->CreateTexture(texSize.x, texSize.y, GFX::TextureFormat::RGBA8, texData.get());

		Xml::Element* charsElement = rootElement->FirstChildElement("chars");

		u32 charCount = 0;
		const Xml::Attribute* charCountAttrib = charsElement->FindAttribute("count");
		charCountAttrib->QueryUnsignedValue(&charCount);

		Glyphs.reserve(static_cast<size_t>(charCount));

		Xml::Element* glyphElement = charsElement->FirstChildElement();
		for (size_t i = 0; i < static_cast<size_t>(charCount); i++)
		{
			if (glyphElement == nullptr)
			{
				break;
			}

			FontGlyph& glyph = Glyphs.emplace_back();

			i32 tempConversionValue = 0; // NOTE: Used for converting into values of types that tinyxml2 does not support natively

			const Xml::Attribute* glyphAttrib = glyphElement->FindAttribute("id");
			glyphAttrib->QueryIntValue(&glyph.CharacterCode);

			glyphAttrib = glyphElement->FindAttribute("x");
			glyphAttrib->QueryIntValue(&tempConversionValue);
			glyph.X = static_cast<u16>(tempConversionValue);

			glyphAttrib = glyphElement->FindAttribute("y");
			glyphAttrib->QueryIntValue(&tempConversionValue);
			glyph.Y = static_cast<u16>(tempConversionValue);

			glyphAttrib = glyphElement->FindAttribute("width");
			glyphAttrib->QueryIntValue(&tempConversionValue);
			glyph.Width = static_cast<u16>(tempConversionValue);

			glyphAttrib = glyphElement->FindAttribute("height");
			glyphAttrib->QueryIntValue(&tempConversionValue);
			glyph.Height = static_cast<u16>(tempConversionValue);

			glyphAttrib = glyphElement->FindAttribute("xoffset");
			glyphAttrib->QueryIntValue(&tempConversionValue);
			glyph.XOffset = static_cast<i8>(tempConversionValue);

			glyphAttrib = glyphElement->FindAttribute("yoffset");
			glyphAttrib->QueryIntValue(&tempConversionValue);
			glyph.YOffset = static_cast<i8>(tempConversionValue);

			glyphAttrib = glyphElement->FindAttribute("xadvance");
			glyphAttrib->QueryIntValue(&tempConversionValue);
			glyph.XAdvance = static_cast<u16>(tempConversionValue);

			glyphElement = glyphElement->NextSiblingElement();
		}

		return true;
	}

	bool Font::ReadBMFont(const std::string_view filePath)
	{
		std::string_view basePath = Path::GetDirectoryPath(filePath);

		std::unique_ptr<u8[]> xmlData;
		size_t xmlSize = File::ReadAllBytes(filePath, xmlData);

		if (xmlData == nullptr || xmlSize == 0)
		{
			return false;
		}

		bool result = ReadBMFont(basePath, xmlData, xmlSize);
		return result;
	}

	void Font::Destroy()
	{
		Texture = nullptr;
		Glyphs.clear();
	}
}
