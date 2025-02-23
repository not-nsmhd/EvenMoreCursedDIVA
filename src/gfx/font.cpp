#include <cstring>
#include <fstream>
#include <new>
#include <tinyxml2.h>
#include "../io/filesystem.h"
#include "helpers/tex_helpers.h"
#include "font.h"

using fsPath = std::filesystem::path;

namespace GFX
{
	Font::Font()
	{
	}
	
	void Font::Destroy()
	{
		backend->DestroyTexture(texture);
		glyphMap.clear();
		glyphs.clear();	
	}

	void Font::LoadBMFont(Backend *backend, const std::filesystem::path &path)
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
		fsPath fntPath = fs->GetContentFilePath(path);

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
						fontGlyph.CharCode = 0xFFFF;
					}
					else
					{
						u16 code = static_cast<u16>(charAttr->IntValue());
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

					if (fontGlyph.CharCode == 0xFFFF)
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

		fsPath texturePath = fntPath;
		texturePath.replace_extension("png");

		texture = LoadImage(backend, texturePath);
		this->backend = backend;
	}

	void Font::PushString(SpriteRenderer &renderer, const string &text, vec2 pos, vec2 scale, Color color)
	{
		if (text.size() == 0)
		{
			return;
		}

		renderer.ResetSprite();

		vec2 charPos = pos;
		vec2 offset = {};

		FontGlyph glyph;
		for (string::const_iterator c = text.begin(); c != text.end(); c++)
		{
			if (*c == '\n')
			{
				charPos.x = pos.x;
				charPos.y += LineHeight;
				continue;
			}

			unordered_map<char16_t, i32>::iterator glyphIt = glyphMap.find(static_cast<char16_t>(*c));
			if (glyphIt == glyphMap.end())
			{
				glyph = replacementGlyph;
			}
			else
			{
				glyph = glyphs[glyphIt->second];
			}

			if (*c == ' ')
			{
				charPos.x += static_cast<float>(glyph.XAdvance);
				continue;
			}

			offset.x = static_cast<float>(glyph.XOffset);
			offset.y = static_cast<float>(glyph.YOffset);

			renderer.SetSpritePosition((charPos + offset) * scale);
			renderer.SetSpriteScale({glyph.Width * scale.x, glyph.Height * scale.y});
			renderer.SetSpriteColor(color);

			renderer.SetSpriteSource(texture,
									 {static_cast<float>(glyph.X),
									  static_cast<float>(glyph.Y),
									  static_cast<float>(glyph.Width),
									  static_cast<float>(glyph.Height)});

			renderer.PushSprite(texture);

			charPos.x += static_cast<float>(glyph.XAdvance);
		}
	}

	void Font::PushString(SpriteRenderer &renderer, const u16string &text, vec2 pos, vec2 scale, Color color)
	{
		if (text.size() == 0)
		{
			return;
		}

		renderer.ResetSprite();

		vec2 charPos = pos;
		vec2 offset = {};

		FontGlyph glyph;

		for (u16string::const_iterator c = text.begin(); c != text.end(); c++)
		{
			if (*c == 0xFEFF)
			{
				continue;
			}

			if (*c == '\n')
			{
				charPos.x = pos.x;
				charPos.y += LineHeight;
				continue;
			}

			unordered_map<char16_t, i32>::iterator glyphIt = glyphMap.find(*c);
			if (glyphIt == glyphMap.end())
			{
				glyph = replacementGlyph;
			}
			else
			{
				glyph = glyphs[glyphIt->second];
			}

			if (*c == ' ')
			{
				charPos.x += static_cast<float>(glyph.XAdvance);
				continue;
			}

			offset.x = static_cast<float>(glyph.XOffset);
			offset.y = static_cast<float>(glyph.YOffset);

			renderer.SetSpritePosition((charPos + offset) * scale);
			renderer.SetSpriteScale({glyph.Width * scale.x, glyph.Height * scale.y});
			renderer.SetSpriteColor(color);

			renderer.SetSpriteSource(texture,
									 {static_cast<float>(glyph.X),
									  static_cast<float>(glyph.Y),
									  static_cast<float>(glyph.Width),
									  static_cast<float>(glyph.Height)});

			renderer.PushSprite(texture);

			charPos.x += static_cast<float>(glyph.XAdvance);
		}
	}

	void Font::PushString(SpriteRenderer &renderer, const char *text, size_t maxLen, vec2 pos, vec2 scale, Color color)
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

		renderer.ResetSprite();

		vec2 charPos = pos;
		vec2 offset = {};

		FontGlyph glyph;
		for (size_t i = 0; i < len; i++)
		{
			char c = text[i];

			if (c == '\n')
			{
				charPos.x = pos.x;
				charPos.y += LineHeight;
				continue;
			}

			unordered_map<char16_t, i32>::iterator glyphIt = glyphMap.find(static_cast<char16_t>(c));
			if (glyphIt == glyphMap.end())
			{
				glyph = replacementGlyph;
			}
			else
			{
				glyph = glyphs[glyphIt->second];
			}

			if (c == ' ')
			{
				charPos.x += static_cast<float>(glyph.XAdvance);
				continue;
			}

			offset.x = static_cast<float>(glyph.XOffset);
			offset.y = static_cast<float>(glyph.YOffset);

			renderer.SetSpritePosition((charPos + offset) * scale);
			renderer.SetSpriteScale({glyph.Width * scale.x, glyph.Height * scale.y});
			renderer.SetSpriteColor(color);

			renderer.SetSpriteSource(texture,
									 {static_cast<float>(glyph.X),
									  static_cast<float>(glyph.Y),
									  static_cast<float>(glyph.Width),
									  static_cast<float>(glyph.Height)});

			renderer.PushSprite(texture);

			charPos.x += static_cast<float>(glyph.XAdvance);
		}
	}

	void Font::PushString(SpriteRenderer &renderer, const char16_t *text, size_t maxLen, vec2 pos, vec2 scale, Color color)
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

		renderer.ResetSprite();

		vec2 charPos = pos;
		vec2 offset = {};

		FontGlyph glyph;
		for (size_t i = 0; i < len; i++)
		{
			char16_t c = text[i];

			if (c == '\n')
			{
				charPos.x = pos.x;
				charPos.y += LineHeight;
				continue;
			}

			unordered_map<char16_t, i32>::iterator glyphIt = glyphMap.find(c);
			if (glyphIt == glyphMap.end())
			{
				glyph = replacementGlyph;
			}
			else
			{
				glyph = glyphs[glyphIt->second];
			}

			if (c == ' ')
			{
				charPos.x += static_cast<float>(glyph.XAdvance);
				continue;
			}

			offset.x = static_cast<float>(glyph.XOffset);
			offset.y = static_cast<float>(glyph.YOffset);

			renderer.SetSpritePosition((charPos + offset) * scale);
			renderer.SetSpriteScale({glyph.Width * scale.x, glyph.Height * scale.y});
			renderer.SetSpriteColor(color);

			renderer.SetSpriteSource(texture,
									 {static_cast<float>(glyph.X),
									  static_cast<float>(glyph.Y),
									  static_cast<float>(glyph.Width),
									  static_cast<float>(glyph.Height)});

			renderer.PushSprite(texture);

			charPos.x += static_cast<float>(glyph.XAdvance);
		}
	}
};
