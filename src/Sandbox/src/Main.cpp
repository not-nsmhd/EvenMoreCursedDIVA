#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GUI/ImGuiTest.h"

using namespace Starshine;

int SDL_main(int argc, char* argv[])
{
	Starshine::GameInstance game;
	
	if (game.Initialize())
	{
		game.GetWindow()->SetTitle("Sandbox");
		game.GetWindow()->SetSize(ivec2(1600, 900));
		game.GetWindow()->CenterWindow();
		game.GetWindow()->SetResizing(true);

		game.SetState(std::make_unique<ImGuiTest>());
		game.EnterLoop();

		return 0;
	}

	return 1;
}
