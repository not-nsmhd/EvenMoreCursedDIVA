#include <SDL2/SDL.h>
#include "Common/Logging/Logging.h"
#include "BuildInfo.h"
#include "GameInstance.h"
#include "Rendering/Device.h"

namespace Starshine
{
	using namespace Rendering;

	struct GameInstance::Impl
	{
		GameInstance& Parent;

		SDL_Window* gameWindow{ nullptr };
		
		bool running{ true };
		SDL_Event winEvent{};

		Device* GFXDevice{ nullptr };
		std::unique_ptr<GameState> CurrentState{};

		Impl(GameInstance& instance) : Parent(instance)
		{
		}

		bool Initialize()
		{
#if defined (_DEBUG)
			LogMessage("--- Starshine %02d.%02d [Debug] ---", BuildInfo::BuildYear - 2000, BuildInfo::BuildMonth);
#else
			LogMessage("--- Starshine %02d.%02d ---", BuildInfo::BuildYear - 2000, BuildInfo::BuildMonth);
#endif

			LogMessage("SDL Platform: %s", SDL_GetPlatform());

			LogMessage("Build Date: %s", BuildInfo::BuildDateString);
			LogMessage("Git Information: %s, %s", BuildInfo::GitBranchName, BuildInfo::GitCommitHashString);

			SDL_Init(SDL_INIT_EVERYTHING);
			gameWindow = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);

			Rendering::InitializeDevice(gameWindow, DeviceType::OpenGL);
			GFXDevice = Rendering::GetDevice();

			return true;
		}

		void Destroy()
		{
			if (CurrentState != nullptr)
			{
				CurrentState->UnloadContent();
				CurrentState->Destroy();
				CurrentState = nullptr;
			}

			Rendering::DestroyDevice();

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

				if (CurrentState != nullptr) { CurrentState->Update(16.667f); }
				if (CurrentState != nullptr) { CurrentState->Draw(16.667f); }
				else
				{
					GFXDevice->Clear(ClearFlags_Color, DefaultColors::Black, 1.0f, 0);
					GFXDevice->SwapBuffers();
				}
			}

			Destroy();
		}

		bool SetState(std::unique_ptr<GameState> state)
		{
			if (CurrentState != nullptr)
			{
				LogMessage("Changing state: [%s] -> [%s]",
					CurrentState->GetStateName().data(), state->GetStateName().data());

				CurrentState->UnloadContent();
				CurrentState->Destroy();
				CurrentState = nullptr;
			}
			else
			{
				LogMessage("Setting initial state: [%s]", state->GetStateName().data());
			}

			if (!state->Initialize()) { return false; }
			LogMessage("Initialized new state");

			if (!state->LoadContent()) { return false; }
			LogMessage("Loaded content of the new state");

			state->GameInstance = &Parent;
			CurrentState = std::move(state);
			return true;
		}
	};

	GameInstance::GameInstance() : impl(std::make_unique<Impl>(*this))
	{
	}

	GameInstance::~GameInstance()
	{
	}

	bool GameInstance::Initialize()
	{
		return impl->Initialize();
	}

	void GameInstance::EnterLoop()
	{
		impl->Loop();
	}

	bool GameInstance::SetState(std::unique_ptr<GameState> state)
	{
		if (state == nullptr) { return false; }
		return impl->SetState(std::move(state));
	}
}
