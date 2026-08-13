#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GameContext.h"

#include "Definitions.h"

using namespace Starshine;
using namespace Sandbox;

int SDL_main(int argc, char* argv[])
{
	Starshine::GameInstance game;
	
	if (game.Initialize(false))
	{
		game.GetWindow()->SetTitle("Sandbox");
		//game.GetWindow()->SetSize(ivec2(1600, 900));
		game.GetWindow()->CenterWindow();
		game.GetWindow()->SetResizing(true);

		Sandbox::GameContext::CreateInstance();

		game.SetState(GetStatePointer<GameState>(StateID::VideoTest));
		game.EnterLoop();

		Sandbox::GameContext::DestroyInstance();
		return 0;
	}

	return 1;
}
