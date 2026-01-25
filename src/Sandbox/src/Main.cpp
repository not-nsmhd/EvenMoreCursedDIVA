#include <SDL2/SDL.h>
#include "GameInstance.h"

using namespace Starshine;

int SDL_main(int argc, char* argv[])
{
	GameInstance game;
	
	if (game.Initialize())
	{
		game.EnterLoop();
		return 0;
	}

	return 1;
}
