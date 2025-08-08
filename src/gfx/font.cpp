#include <cstring>
#include <fstream>
#include <utf8.h>
#include <tinyxml2.h>
#include "../io/filesystem.h"
#include "helpers/tex_helpers.h"
#include "font.h"

namespace GFX
{
	using namespace std;
	using namespace std::filesystem;
	using namespace LowLevel;
	using namespace Common;

	Font::Font()
	{
	}
	
	void Font::Destroy()
	{
		backend->DestroyTexture(texture);
		glyphMap.clear();
		glyphs.clear();	
	}

	void Font::LoadBMFont(Backend *backend, const path& filePath)
	{
		if (backend == nullptr)
		{
			return;
		}

		using namespace std;
		using namespace tinyxml2;
		using namespace GFX::Helpers;
		using namespace IO;

		FileSystem* fs = FileSystem::GetInstance();
		path fntPath = fs->GetContentFilePath(filePath);

		ifstream xmlFile;
		xmlFile.open(fntPath, ios::in | ios::binary);

		if (xmlFile.bad())
		{
			return;
		}

		// Get file size
		size_t xmlSize = std::filesystem::file_size(fntPath);
		char *xmlData = new char[xmlSize + 1];
		xmlData[xmlSize] = '\0';

		xmlFile.read(xmlData, xmlSize);
		xmlFile.close();

		XMLDocument doc;
		doc.Parse(xmlData, xmlSize);

		XMLElement* rootElement = doc.FirstChildElement("font");

		for (XMLElement* element = rootElement->FirstChildElement(); element; element = element->NextSiblingElement())
		{
			if (strncmp(element->Name(), "info", 5) == 0)
			{
				const XMLAttribute* faceAttr = element->FindAttribute("face"); 
				Typeface = faceAttr->Value();
			}

			if (strncmp(element->Name(), "common", 8) == 0)
			{
				const XMLAttribute* lineHeight_Attr = element->FindAttribute("lineHeight"); 
				LineHeight = lineHeight_Attr->FloatValue();
			}

			if (strncmp(element->Name(), "chars", 5) == 0)
			{
				const XMLAttribute* countAttr = element->FindAttribute("count"); 
				size_t count = static_cast<size_t>(countAttr->IntValue());

				size_t charIndex = 0;
				const XMLAttribute* charAttr;
				FontGlyph fontGlyph;

				for (XMLElement* charNode = element->FirstChildElement(); charNode; charNode = charNode->NextSiblingElement())
				{
					if (charIndex + 1 >= count)
					{
						break;
					}

					charAttr = charNode->FindAttribute("id");
					if (charAttr->IntValue() == -1)
					{
						fontGlyph.CharCode = ReplacementCharcode;
					}
					else
					{
						u32 code = static_cast<u32>(charAttr->IntValue());
						fontGlyph.CharCode = code;
					}

					charAttr = charNode->FindAttribute("x");
					fontGlyph.X = static_cast<u16>(charAttr->IntValue());

					charAttr = charNode->FindAttribute("y");
					fontGlyph.Y = static_cast<u16>(charAttr->IntValue());

					charAttr = charNode->FindAttribute("width");
					fontGlyph.Width = static_cast<u16>(charAttr->IntValue());

					charAttr = charNode->FindAttribute("height");
					fontGlyph.Height = static_cast<u16>(charAttr->IntValue());

					charAttr = charNode->FindAttribute("xoffset");
					fontGlyph.XOffset = static_cast<i16>(charAttr->IntValue());

					charAttr = charNode->FindAttribute("yoffset");
					fontGlyph.YOffset = static_cast<i16>(charAttr->IntValue());

					charAttr = charNode->FindAttribute("xadvance");
					fontGlyph.XAdvance = static_cast<i16>(charAttr->IntValue());

					if (fontGlyph.CharCode == ReplacementCharcode)
					{
						replacementGlyph = fontGlyph;
					}
					else
					{
						glyphs.push_back(fontGlyph);
						glyphMap[fontGlyph.CharCode] = charIndex++;
					}
				}
			}
		}

		doc.Clear();
		delete[] xmlData;

		path texturePath = fntPath;
		texturePath.replace_extension("png");

		texture = LoadTexture(backend, texturePath);
		string texName = texturePath.filename().replace_extension("").generic_string() + "_FontTexture";
		texture->SetDebugName(texName.c_str());
		this->backend = backend;
	}

	void Font::PushString(SpriteRenderer* renderer, string_view text, vec2 pos, vec2 scale, Color color)
	{
		if (text.size() == 0)
		{
			return;
		}

		renderer->ResetSprite();

		vec2 charPos = {};

		for (string_view::const_iterator c = text.begin(); c != text.end(); c++)
		{
			char16_t c16 = static_cast<char16_t>(*c);
			PushChar(renderer, c16, pos, &charPos, scale, color);
		}
	}

	void Font::PushString(SpriteRenderer* renderer, u16string_view text, vec2 pos, vec2 scale, Color color)
	{
		if (text.size() == 0)
		{
			return;
		}

		renderer->ResetSprite();

		vec2 charPos = {};

		for (u16string_view::const_iterator c = text.begin(); c != text.end(); c++)
		{
			if (*c == 0xFEFF)
			{
				// Skip BOM
				continue;
			}

			PushChar(renderer, *c, pos, &charPos, scale, color);
		}
	}

	void Font::PushString(SpriteRenderer* renderer, const char *text, size_t maxLen, vec2 pos, vec2 scale, Color color)
	{
		if (text == nullptr || maxLen == 0)
		{
			return;
		}

		size_t len = 0;
		for (size_t i = 0; i < maxLen; i++)
		{
			if (text[i] == '\0')
			{
				break;
			}
			len++;
		}

		renderer->ResetSprite();

		vec2 charPos = {};

		for (size_t i = 0; i < len; i++)
		{
			char16_t c16 = static_cast<char16_t>(text[i]);
			PushChar(renderer, c16, pos, &charPos, scale, color);
		}
	}

	void Font::PushString(SpriteRenderer* renderer, const char16_t *text, size_t maxLen, vec2 pos, vec2 scale, Color color)
	{
		if (text == nullptr || maxLen == 0)
		{
			return;
		}

		size_t len = 0;
		for (size_t i = 0; i < maxLen; i++)
		{
			if (text[i] == 0)
			{
				break;
			}
			len++;
		}

		renderer->ResetSprite();

		vec2 charPos = {};

		for (size_t i = 0; i < len; i++)
		{
			char16_t c = text[i];
			PushChar(renderer, c, pos, &charPos, scale, color);
		}
	}

	void Font::PushUTF8String(SpriteRenderer* renderer, const u8 *text, size_t arraySize, vec2 pos, vec2 scale, Color color)
	{
		if (text == nullptr || arraySize == 0)
		{
			return;
		}

		const u8* t = text;
		size_t len = 0;
		while (t < &text[arraySize - 1])
		{
			if (utf8::next(t, &text[arraySize - 1]) != 0)
			{
				len++;
			}
		}

		t = text;
		renderer->ResetSprite();

		vec2 charPos = {};

		for (size_t i = 0; i < len; i++)
		{
			u32 c = utf8::next(t, &text[arraySize - 1]);
			PushChar(renderer, c, pos, &charPos, scale, color);
		}
	}
	
	void Font::PushChar(SpriteRenderer* renderer, u32 c, vec2 basePos, vec2* charPos, vec2 scale, Color color)
	{
		if (c == '\n')
		{
			charPos->x = 0.0f;
			charPos->y += LineHeight;
			return;
		}

		unordered_map<u32, i32>::iterator glyphIt = glyphMap.find(c);

		const FontGlyph* glyph;
		if (glyphIt == glyphMap.end())
		{
			glyph = &replacementGlyph;
		}
		else
		{
			glyph = &glyphs[glyphIt->second];
		}

		if (c == ' ')
		{
			charPos->x += static_cast<float>(glyph->XAdvance);
			return;
		}

		vec2 offset = vec2(static_cast<float>(glyph->XOffset), static_cast<float>(glyph->YOffset));

		renderer->SetSpritePosition((*charPos + basePos + offset) * scale);
		renderer->SetSpriteScale(vec2 {glyph->Width * scale.x, glyph->Height * scale.y});
		renderer->SetSpriteColor(color);

		renderer->SetSpriteSource(texture, RectangleF 
			{ 
				static_cast<float>(glyph->X), 
				static_cast<float>(glyph->Y),
				static_cast<float>(glyph->Width), 
				static_cast<float>(glyph->Height)
			});

		renderer->PushSprite(texture);

		charPos->x += static_cast<float>(glyph->XAdvance);
	}
};
