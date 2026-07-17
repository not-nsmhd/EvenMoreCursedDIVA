#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GameContext.h"
#include "GUI/ImGuiTest.h"
#include "Graphics/FontTest.h"

#include "Definitions.h"

using namespace Starshine;

int SDL_main(int argc, char* argv[])
{
	Starshine::GameInstance game;
	
	if (game.Initialize())
	{
		game.GetWindow()->SetTitle("Sandbox");
		//game.GetWindow()->SetSize(ivec2(1600, 900));
		game.GetWindow()->CenterWindow();
		game.GetWindow()->SetResizing(true);

		Sandbox::GameContext::CreateInstance();

		game.RegisterState<ImGuiTest>();
		game.RegisterState<Sandbox::FontTest>();

		game.SetState(Sandbox::GameState_FontTest);
		game.EnterLoop();

		Sandbox::GameContext::DestroyInstance();
		return 0;
	}

	return 1;
}
