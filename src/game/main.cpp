#include <SDL2/SDL.h>
#include "util/logging.h"
#include "game_new.h"

#ifdef __cplusplus
extern "C"
{
#endif
	int SDL_main(int argc, char** argv)
	{
#ifdef _DEBUG
		Logging::ToggleLoggingToConsole(true);
		Logging::ToggleLoggingToFile(true);
#endif

		Logging::LoggingInit();

		Starshine::Game game;
		int returnCode = game.Run();

		Logging::LoggingQuit();
		return returnCode;
	}
#ifdef __cplusplus
}
#endif
