#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GameContext.h"
#include "Definitions.h"

#include "Menu/ChartSelect.h"
#include "MainGame/MainGame.h"

using namespace Starshine;
using namespace DIVA;

int SDL_main(int argc, char* argv[])
{
	GameInstance game;
	
	if (game.Initialize())
	{
		game.GetWindow()->SetTitle("Even More Cursed DIVA");
		//game.GetWindow()->SetResizing(true);

		if (!GameContext::CreateInstance()) { return 1; }

		game.RegisterState<Menu::ChartSelect>();
		game.RegisterState<MainGame::MainGameState>();

		if (!game.SetState(GameState_ChartSelect)) { return 1; }

		game.EnterLoop();
		return 0;
	}

	return 1;
}
