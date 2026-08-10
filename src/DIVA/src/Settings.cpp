#include "Settings.h"
#include <IO/Xml.h>
#include <IO/Path/File.h>

using namespace Starshine;
using namespace Starshine::Input;

namespace DIVA
{
	namespace ElementNames
	{
		static constexpr const char* Settings = "Settings";

		static constexpr const char* Window = "Window";
		static constexpr const char* Audio = "Audio";
		static constexpr const char* Input = "Input";

		static constexpr const char* Window_Mode = "Mode";
		static constexpr const char* Window_Position = "Position";
		static constexpr const char* Window_Size = "Size";
		static constexpr const char* Window_Maximized = "Maximized";

		static constexpr const char* Audio_MusicVolume = "MusicVolume";
		static constexpr const char* Audio_SoundVolume = "SoundVolume";
	}

	bool Settings::LoadFromFile(std::string_view filePath)
	{
		bool parseResult = true;
		if (!IO::File::Exists(filePath)) return false;

		Xml::Document document;
		if (!Xml::ParseFromFile(document, filePath)) return false;

		Xml::Element* rootElement = document.FirstChildElement(ElementNames::Settings);
		if (rootElement == nullptr) return false;

		// -----------------

		Xml::Element* windowElement = rootElement->FirstChildElement(ElementNames::Window);
		if (windowElement == nullptr) return false;

		const char* windowModeString = windowElement->Attribute(ElementNames::Window_Mode);
		for (size_t i = 0; i < EnumCount<WindowMode>(); i++)
		{
			const char* testValue = WindowModeNames[i];
			if (SDL_strncmp(windowModeString, testValue, SDL_strlen(testValue) + 1) == 0)
			{
				Window.Mode = static_cast<WindowMode>(i);
				break;
			}
			else
				continue;
		}

		parseResult = Xml::TryGetValue(Window.Position, windowElement->FindAttribute(ElementNames::Window_Position));
		parseResult = Xml::TryGetValue(Window.Size, windowElement->FindAttribute(ElementNames::Window_Size));
		parseResult = Xml::TryGetValue(Window.Maximized, windowElement->FindAttribute(ElementNames::Window_Maximized));

		// -----------------

		Xml::Element* audioElement = rootElement->FirstChildElement(ElementNames::Audio);
		if (audioElement == nullptr) return false;

		parseResult = Xml::TryGetValue(Audio.MusicVolume, audioElement->FindAttribute(ElementNames::Audio_MusicVolume));
		parseResult = Xml::TryGetValue(Audio.SoundVolume, audioElement->FindAttribute(ElementNames::Audio_SoundVolume));

		// -----------------
		
		Xml::Element* inputElement = rootElement->FirstChildElement(ElementNames::Input);
		if (inputElement == nullptr) return false;

		const auto readKeybindElement = [&](Xml::Element* parentElement, const char* name, KeyBind& keybind)
		{
			Xml::Element* keybindElement = parentElement->FirstChildElement(name);
			if (keybindElement == nullptr) return false;

			if (!Xml::TryGetValue(keybind.Primary, keybindElement->FindAttribute("Primary")))
				return false;

			const Xml::Attribute* secondaryAttrib = keybindElement->FindAttribute("Secondary");
			if (secondaryAttrib != nullptr)
			{
				if (!Xml::TryGetValue(keybind.Secondary, secondaryAttrib))
					return false;
			}

			return true;
		};

		parseResult = readKeybindElement(inputElement, "MainGame_Triangle", Input.MainGame_Triangle);
		parseResult = readKeybindElement(inputElement, "MainGame_Circle", Input.MainGame_Circle);
		parseResult = readKeybindElement(inputElement, "MainGame_Cross", Input.MainGame_Cross);
		parseResult = readKeybindElement(inputElement, "MainGame_Square", Input.MainGame_Square);
		parseResult = readKeybindElement(inputElement, "MainGame_Star", Input.MainGame_Star);

		// -----------------

		return parseResult;
	}

	void Settings::SaveToFile(std::string_view filePath)
	{
		Xml::Document document;
		Xml::Element* rootElement = document.NewElement(ElementNames::Settings);

		Xml::Element* windowElement = rootElement->InsertNewChildElement(ElementNames::Window);
		windowElement->SetAttribute(ElementNames::Window_Mode, WindowModeNames[static_cast<size_t>(Window.Mode)]);
		Xml::SetAttribute(windowElement, ElementNames::Window_Position, Window.Position);
		Xml::SetAttribute(windowElement, ElementNames::Window_Size, Window.Size);
		Xml::SetAttribute(windowElement, ElementNames::Window_Maximized, Window.Maximized);

		Xml::Element* audioElement = rootElement->InsertNewChildElement(ElementNames::Audio);
		audioElement->SetAttribute(ElementNames::Audio_MusicVolume, Audio.MusicVolume);
		audioElement->SetAttribute(ElementNames::Audio_SoundVolume, Audio.SoundVolume);

		Xml::Element* inputElement = rootElement->InsertNewChildElement(ElementNames::Input);
	
		const auto writeKeybindElement = [&](Xml::Element* parentElement, const char* name, KeyBind keybind)
		{
			Xml::Element* keybindElement = parentElement->InsertNewChildElement(name);
			keybindElement->SetAttribute("Primary", keybind.Primary);
			if (keybind.Secondary != UnboundKey)
				keybindElement->SetAttribute("Secondary", keybind.Secondary);
		};

		writeKeybindElement(inputElement, "MainGame_Triangle", Input.MainGame_Triangle);
		writeKeybindElement(inputElement, "MainGame_Circle", Input.MainGame_Circle);
		writeKeybindElement(inputElement, "MainGame_Cross", Input.MainGame_Cross);
		writeKeybindElement(inputElement, "MainGame_Square", Input.MainGame_Square);
		writeKeybindElement(inputElement, "MainGame_Star", Input.MainGame_Triangle);

		document.InsertFirstChild(rootElement);
		document.SaveFile(filePath.data());
	}

	void Settings::SetDefaultValues()
	{
		Window.Mode = WindowMode::Window;
		Window.Position = CenteredWindowPos;
		Window.Size = ivec2(1280, 720);
		Window.Maximized = false;

		Audio.MusicVolume = 70;
		Audio.SoundVolume = 70;

		Input.MainGame_Triangle = KeyBind(SDLK_i, SDLK_w);
		Input.MainGame_Circle = KeyBind(SDLK_l, SDLK_d);
		Input.MainGame_Cross = KeyBind(SDLK_k, SDLK_s);
		Input.MainGame_Square = KeyBind(SDLK_j, SDLK_a);
		Input.MainGame_Star = KeyBind(SDLK_f, SDLK_h);
	}
}
