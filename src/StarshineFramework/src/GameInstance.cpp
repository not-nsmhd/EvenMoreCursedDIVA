#include "GameInstance.h"
#include <SDL2/SDL.h>

namespace Starshine
{
	struct GameInstance::Impl
	{
		SDL_Window* gameWindow{ nullptr };
		
		bool running{ true };
		SDL_Event winEvent{};

		bool Initialize()
		{
			SDL_Init(SDL_INIT_EVERYTHING);
			gameWindow = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
			return true;
		}

		void Destroy()
		{
			SDL_DestroyWindow(gameWindow);
			SDL_Quit();
		}

		void Loop()
		{
			while (running)
			{
				if (SDL_PollEvent(&winEvent))
				{
					switch (winEvent.type)
					{
					case SDL_QUIT:
						running = false;
						break;
					}
				}
			}
		}
	};

	GameInstance::GameInstance() : impl(std::make_unique<Impl>())
	{
	}

	GameInstance::~GameInstance()
	{
		impl->Destroy();
	}

	bool GameInstance::Initialize()
	{
		return impl->Initialize();
	}

	void GameInstance::EnterLoop()
	{
		impl->Loop();
	}
}
