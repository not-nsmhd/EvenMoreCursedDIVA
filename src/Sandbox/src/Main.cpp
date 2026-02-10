#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "Graphics/GraphicsTest.h"

int SDL_main(int argc, char* argv[])
{
	Starshine::GameInstance game;
	
	if (game.Initialize())
	{
		game.GetWindow()->SetTitle("Sandbox");
		//game.GetWindow()->SetResizing(true);

		game.SetState(std::make_unique<TestState>());
		game.EnterLoop();

		return 0;
	}

	return 1;
}
