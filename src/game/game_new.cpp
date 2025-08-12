#include "game_new.h"
#include <SDL2/SDL.h>

namespace Starshine
{
	struct Game::Impl
	{
		SDL_Window* GameWindow{};

		bool Running{};
		SDL_Event SDLEvent{};

		bool Initialize()
		{
			SDL_Init(SDL_INIT_EVERYTHING);
			GameWindow = SDL_CreateWindow("Starshine Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);

			Running = true;

			return true;
		}

		void Destroy()
		{
			SDL_DestroyWindow(GameWindow);
			SDL_Quit();
		}

		void Loop()
		{
			while (Running)
			{
				if (SDL_PollEvent(&SDLEvent) != 0)
				{
					switch (SDLEvent.type)
					{
					case SDL_QUIT:
						Running = false;
						break;
					}
				}
			}

			Destroy();
		}
	};

	Game::Game() : impl(new Impl())
	{
	}

	int Game::Run()
	{
		if (impl->Initialize())
		{
			impl->Loop();
			return EXIT_SUCCESS;
		}
		return EXIT_FAILURE;
	}
}
