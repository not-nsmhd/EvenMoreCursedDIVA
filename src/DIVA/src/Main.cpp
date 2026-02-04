#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "Menu/ChartSelect.h"

using namespace Starshine;

int SDL_main(int argc, char* argv[])
{
	GameInstance game;
	
	if (game.Initialize())
	{
		game.SetState(std::make_unique<DIVA::Menu::ChartSelect>());
		game.EnterLoop();
		return 0;
	}

	return 1;
}
