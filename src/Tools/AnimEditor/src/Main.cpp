#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GameContext.h"

#include "Definitions.h"
#include "Editor/AnimEditor.h"

using namespace Starshine;

int SDL_main(int argc, char* argv[])
{
	Starshine::GameInstance game;

	if (game.Initialize(true))
	{
		game.GetWindow()->SetTitle("Animation Editor");
		game.GetWindow()->SetSize(ivec2(1600, 900));
		game.GetWindow()->CenterWindow();
		game.GetWindow()->SetResizing(true);

		GameContext::CreateInstance();

		if (!game.SetState(GetStatePointer(StateID::AnimEditor))) { return 1; }
		game.EnterLoop();

		GameContext::DestroyInstance();
		return 0;
	}

	return 1;
}
