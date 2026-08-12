#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GameContext.h"
#include "Definitions.h"
#include "Settings.h"
#include <Common/Logging/Logging.h>
#include <IO/Path/File.h>

using namespace Starshine;
using namespace DIVA;

bool ConvertFont(std::string_view inputFilePath, std::string_view outputFilePath, Graphics::TextureFormat texFormat)
{
	Graphics::Font font;
	if (!font.ReadBMFont(inputFilePath, texFormat))
		return false;

	IO::FileStream outputFileStream = IO::File::CreateWrite(outputFilePath);
	IO::StreamWriter writer(outputFileStream);

	font.WriteBinary(writer);

	return true;
}

int SDL_main(int argc, char* argv[])
{
	if (argc >= 2)
	{
		if (!SDL_strncmp(argv[1], "--convert_font", 32))
		{
			if (argc < 4)
				return 1;

			Graphics::TextureFormat targetFormat = Graphics::TextureFormat::RGBA8;
			if (argc >= 5)
			{
				for (size_t i = 0; i < Graphics::TextureFormatNames.size(); i++)
				{
					if (!SDL_strncmp(argv[4], Graphics::TextureFormatNames[i].data(), Graphics::TextureFormatNames[i].size()))
					{
						targetFormat = static_cast<Graphics::TextureFormat>(i);
						break;
					}
				}
			}

			ConvertFont(argv[2], argv[3], targetFormat);
			return 0;
		}
		return 0;
	}

	GameInstance game;
	
	if (game.Initialize(false))
	{
		if (!GameContext::CreateInstance()) { return 1; }

		if (!SettingsData.LoadFromFile())
			SettingsData.SetDefaultValues();

		auto window = game.GetWindow();
		window->SetTitle("Even More Cursed DIVA");
		window->SetMode(SettingsData.Window.Mode);
		window->SetPosition(SettingsData.Window.Position);
		window->SetSize(SettingsData.Window.Size);

		if (!game.SetState(GetStatePointer(StateID::ChartSelect))) { return 1; }

		game.EnterLoop();

		SettingsData.Window.Mode = window->GetMode();
		SettingsData.Window.Position = window->GetPosition();
		SettingsData.Window.Size = window->GetSize();
		SettingsData.SaveToFile();

		GameContext::GetInstance()->Unload();
		game.Destroy();
		return 0;
	}

	return 1;
}
