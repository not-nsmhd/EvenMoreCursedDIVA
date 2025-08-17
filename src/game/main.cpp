#include <SDL2/SDL.h>
#include "util/logging.h"
#include "game.h"

using Starshine::Game;

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

		Game::CreateInstance();
		Game& gameInstance = Game::GetInstance();

		int returnCode = gameInstance.Run();
		Game::DeleteInstance();

		Logging::LoggingQuit();
		return returnCode;
	}
#ifdef __cplusplus
}
#endif
