#include <locale>
#include "util/logging.h"
#include "game.h"

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
		if (game.Initialize() != true)
		{
			Logging::LoggingQuit();
			return -1;
		}

		game.SetState(GameStates::Dev_StateSelector); 
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
