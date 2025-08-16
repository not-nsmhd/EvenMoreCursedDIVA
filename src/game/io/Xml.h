#pragma once
#include "../common/types.h"
#include "../common/color.h"
#include <SDL2/SDL_stdinc.h>
#include <tinyxml2.h>

namespace Xml
{
	constexpr std::string_view Extension = "xml";

	using Document = tinyxml2::XMLDocument;
	using Element = tinyxml2::XMLElement;
	using Attribute = tinyxml2::XMLAttribute;

	inline void Parse(Document& doc, char* text, size_t textSize) { doc.Parse(text, textSize); }

	inline Element* GetRootElement(Document& doc) { return doc.RootElement(); }
	inline const Element* GetRootElement(const Document& doc) { return doc.RootElement(); }

	inline Element* FindElement(Element* element, std::string_view name) { return element->FirstChildElement(name.data()); }
	inline const Element* FindElement(const Element* element, std::string_view name) { return element->FirstChildElement(name.data()); }

	inline const Attribute* FindAttribute(Element* element, std::string_view name) { return element->FindAttribute(name.data()); }
	inline const Attribute* FindAttribute(const Element* element, std::string_view name) { return element->FindAttribute(name.data()); }

	inline size_t GetNameLength(Element* element) { size_t len = SDL_strlen(element->Name()); return SDL_min(len, 256); }
	inline size_t GetNameLength(const Element* element) { size_t len = SDL_strlen(element->Name()); return SDL_min(len, 256); }

	inline size_t GetNameLength(Attribute* attrib) { size_t len = SDL_strlen(attrib->Name()); return SDL_min(len, 256); }
	inline size_t GetNameLength(const Attribute* attrib) { size_t len = SDL_strlen(attrib->Name()); return SDL_min(len, 256); }

	inline size_t GetValueLength(Element* element) { return SDL_strlen(element->Value()); }
	inline size_t GetValueLength(const Element* element) { return SDL_strlen(element->Value()); }

	inline size_t GetValueLength(Attribute* attrib) { return SDL_strlen(attrib->Value()); }
	inline size_t GetValueLength(const Attribute* attrib) { return SDL_strlen(attrib->Value()); }

	inline Common::Color TryGetHexColor(const Attribute* attrib)
	{
		if (attrib != nullptr && GetValueLength(attrib) > 1)
		{
			const char* value = attrib->Value();
			if (*value == '#')
			{
				value++;
			}

			u32 r = 0, g = 0, b = 0, a = 0;
			SDL_sscanf(value, "%02x%02x%02x%02x", &r, &g, &b, &a);
			return Common::Color{ static_cast<u8>(r), static_cast<u8>(g), static_cast<u8>(b), static_cast<u8>(a) };
		}
		else
		{
			Common::Color{};
		}
	}
}
