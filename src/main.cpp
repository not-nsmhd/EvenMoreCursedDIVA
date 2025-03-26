#include <locale>
#include "util/logging.h"
#include "game.h"

#include "main_game/main_game.h"
#include "testing/u16_test.h"
#include "testing/input_test.h"
#include "dev/state_selector.h"

#ifdef __cplusplus
extern "C"
{
#endif
	int main(int argc, char** argv)
	{
		std::locale::global(std::locale::classic());

#ifdef _DEBUG
		Logging::ToggleLoggingToConsole(true);
		Logging::ToggleLoggingToFile(true);
#endif

		Logging::LoggingInit();

		if (argc > 1)
		{
			char* arg = nullptr;
			char* longParamName = nullptr;

			for (int i = 0; i < argc; i++)
			{
				arg = argv[i];

				if (SDL_strncmp(arg, "--", 2) == 0)
				{
					longParamName = &arg[2];

					if (SDL_strncmp(longParamName, "log-to-file", 12) == 0)
					{
						Logging::ToggleLoggingToFile(true);
					}
				}
			}
		}

		Game game;
		game.stateList[static_cast<int>(GameStates::STATE_MAINGAME)] = MainGame::MainGameState::GetInstance();

		game.stateList[static_cast<int>(GameStates::DEVSTATE_U16_TEST)] = Testing::U16Test::GetInstance();
		game.stateList[static_cast<int>(GameStates::DEVSTATE_INPUT_TEST)] = Testing::InputTest::GetInstance();
		
		game.stateList[static_cast<int>(GameStates::DEVSTATE_STATE_SELECTOR)] = Dev::StateSelector::GetInstance();

		if (game.Initialize() != true)
		{
			Logging::LoggingQuit();
			return -1;
		}

		game.SetState(GameStates::DEVSTATE_STATE_SELECTOR); 
		if (game.Loop() != true)
		{
			Logging::LoggingQuit();
			return -1;
		}

		Logging::LoggingQuit();
		return 0;
	}
#ifdef __cplusplus
}
#endif
