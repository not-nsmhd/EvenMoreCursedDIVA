#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GameContext.h"
#include "Definitions.h"

using namespace Starshine;
using namespace DIVA;

int SDL_main(int argc, char* argv[])
{
	GameInstance game;
	
	if (game.Initialize(false))
	{
		game.GetWindow()->SetTitle("Even More Cursed DIVA");

		if (!GameContext::CreateInstance()) { return 1; }
		if (!game.SetState(GetStatePointer(StateID::ChartSelect))) { return 1; }

		game.EnterLoop();

		GameContext::GetInstance()->Unload();
		game.Destroy();
		return 0;
	}

	return 1;
}
