#pragma once
#include <Common/Types.h>
#include <Common/MathExt.h>
#include <Common/Color.h>
#include <SDL2/SDL_stdinc.h>
#include <tinyxml2.h>

namespace Starshine::Xml
{
	constexpr std::string_view Extension = "xml";

	using Document = tinyxml2::XMLDocument;
	using Printer = tinyxml2::XMLPrinter;
	using Element = tinyxml2::XMLElement;
	using Attribute = tinyxml2::XMLAttribute;
	using Node = tinyxml2::XMLNode;
	using Text = tinyxml2::XMLText;
	using Error = tinyxml2::XMLError;

	inline bool Parse(Document& doc, const char* text, size_t textSize) { return doc.Parse(text, textSize) == Error::XML_SUCCESS; }
	inline bool ParseFromFile(Document& doc, std::string_view filePath)
	{ 
		return doc.LoadFile(filePath.data()) == Error::XML_SUCCESS;
	}

	inline Element* GetRootElement(Document& doc) { return doc.RootElement(); }
	inline const Element* GetRootElement(const Document& doc) { return doc.RootElement(); }

	inline Element* FindElement(Element* element, std::string_view name)
	{
		if (SDL_strncmp(element->Name(), name.data(), name.size()) == 0) { return element; }
		return element->FirstChildElement(name.data());
	}

	inline const Element* FindElement(const Element* element, std::string_view name)
	{
		if (SDL_strncmp(element->Name(), name.data(), name.size()) == 0) { return element; }
		return element->FirstChildElement(name.data());
	}

	inline const Attribute* FindAttribute(Element* element, std::string_view name) { return element->FindAttribute(name.data()); }
	inline const Attribute* FindAttribute(const Element* element, std::string_view name) { return element->FindAttribute(name.data()); }

	inline size_t GetNameLength(Element* element) { size_t len = SDL_strlen(element->Name()); return MathExtensions::Min(len, 256ULL); }
	inline size_t GetNameLength(const Element* element) { size_t len = SDL_strlen(element->Name()); return MathExtensions::Min(len, 256ULL); }

	inline size_t GetNameLength(Attribute* attrib) { size_t len = SDL_strlen(attrib->Name()); return MathExtensions::Min(len, 256ULL); }
	inline size_t GetNameLength(const Attribute* attrib) { size_t len = SDL_strlen(attrib->Name()); return MathExtensions::Min(len, 256ULL); }

	inline size_t GetValueLength(Element* element) { return SDL_strlen(element->Value()); }
	inline size_t GetValueLength(const Element* element) { return SDL_strlen(element->Value()); }

	inline size_t GetValueLength(Attribute* attrib) { return SDL_strlen(attrib->Value()); }
	inline size_t GetValueLength(const Attribute* attrib) { return SDL_strlen(attrib->Value()); }

	static constexpr std::string_view DefaultColorFormat = "%02x%02x%02x%02x";
	static constexpr std::string_view DefaultVec2Format = "%.8g %.8g";

	inline Color TryGetHexColor(const Attribute* attrib)
	{
		if (attrib != nullptr && GetValueLength(attrib) > 1)
		{
			const char* value = attrib->Value();

			if (*value == '#')
				value++;

			u32 r = 0, g = 0, b = 0, a = 0;
			sscanf_s(value, DefaultColorFormat.data(), &r, &g, &b, &a);
			return Color{ static_cast<u8>(r), static_cast<u8>(g), static_cast<u8>(b), static_cast<u8>(a) };
		}

		return Color{};
	}

	inline void SetAttribute(Element* element, std::string_view name, const vec2& value)
	{
		if (!name.empty())
		{
			char valueString[32]{};
			SDL_snprintf(valueString, sizeof(valueString) - 1, DefaultVec2Format.data(), value.x, value.y);
			element->SetAttribute(name.data(), valueString);
		}
	}

	inline void SetAttribute(Element* element, std::string_view name, const Color& value)
	{
		if (!name.empty())
		{
			char hexColorString[32]{};
			SDL_snprintf(hexColorString, sizeof(hexColorString) - 1, DefaultColorFormat.data(), value.R, value.G, value.B, value.A);
			element->SetAttribute(name.data(), hexColorString);
		}
	}
}
