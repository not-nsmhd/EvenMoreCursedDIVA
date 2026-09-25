#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GameContext.h"
#include "Definitions.h"
#include "Settings.h"
#include <Common/Logging/Logging.h>
#include <IO/FileSystem/FileSystem.h>

using namespace Starshine;
using namespace DIVA;

#ifdef __cplusplus
extern "C"
{
#endif
	int SDL_main(int argc, char* argv[])
	{
		GameInstance game;

		if (game.Initialize(false))
		{
			if (!IO::FileSystem::GetInstance()->AddSearchPath("diva"))
				return 1;

			if (!GameContext::CreateInstance())
				return 1;

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
#ifdef __cplusplus
}
#endif
