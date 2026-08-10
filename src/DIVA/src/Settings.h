#pragma once
#include <Common/Types.h>
#include <Window.h>
#include <Input/Keyboard.h>

namespace DIVA
{
	constexpr std::string_view SettingsFilePath = "userdata/settings.xml";

	struct Settings
	{
		bool LoadFromFile(std::string_view filePath = SettingsFilePath);
		void SaveToFile(std::string_view filePath = SettingsFilePath);
		void SetDefaultValues();

		struct
		{
			Starshine::WindowMode Mode{};
			ivec2 Position{};
			ivec2 Size{};
			bool Maximized{};
		} Window;
		
		// TOOD: Implement
		struct
		{
			i32 MusicVolume{};
			i32 SoundVolume{};
		} Audio;

		struct
		{
			Starshine::Input::KeyBind MainGame_Triangle{};
			Starshine::Input::KeyBind MainGame_Circle{};
			Starshine::Input::KeyBind MainGame_Cross{};
			Starshine::Input::KeyBind MainGame_Square{};
			Starshine::Input::KeyBind MainGame_Star{};
		} Input;

		// TODO: Implement gamepad bind settings
	};

	inline Settings SettingsData = {};
}
