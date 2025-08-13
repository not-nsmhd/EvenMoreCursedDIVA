#include "game_new.h"
#include "build/build_info.h"
#include "util/logging.h"
#include <SDL2/SDL.h>
#include "gfx/new/Renderer.h"

namespace Starshine
{
	using namespace Logging;
	using namespace GFX;

	using Common::Color;

	constexpr const char* LogName = "Starshine::Game";
	constexpr u64 DefaultTargetFrameTime_Ticks = 166667;

	Game* GameInstance = nullptr;

	class TestGameState;
	class TestGameState2;

	class TestGameState : public GameState
	{
		Renderer* GFXRenderer = nullptr;
		f64 ElapsedTime = 0.0;

		bool Initialize()
		{
			GFXRenderer = &Renderer::GetInstance();
			return true;
		}

		bool LoadContent()
		{
			return true;
		}

		void UnloadContent()
		{
		}

		void Destroy()
		{
			GFXRenderer = nullptr;
		}

		void Update(f64 deltaTime_milliseconds)
		{
			ElapsedTime += deltaTime_milliseconds / 1000.0;

			if (ElapsedTime >= 10.0)
			{
				GameState* newState = GameStateHelpers::CreateGameStateInstance<TestGameState2>();
				Game::GetInstance().SetCurrentGameState(newState);
			}
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			GFXRenderer->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);
			GFXRenderer->SwapBuffers();
		}

		std::string_view GetStateName() const
		{
			return "Testing Game State";
		}
	};

	class TestGameState2 : public GameState
	{
		Renderer* GFXRenderer = nullptr;
		f64 ElapsedTime = 0.0;

		bool Initialize()
		{
			GFXRenderer = &Renderer::GetInstance();
			return true;
		}

		bool LoadContent()
		{
			return true;
		}

		void UnloadContent()
		{
		}

		void Destroy()
		{
		}

		void Update(f64 deltaTime_milliseconds)
		{
			ElapsedTime += deltaTime_milliseconds / 1000.0;

			if (ElapsedTime >= 10.0)
			{
				GameState* newState = GameStateHelpers::CreateGameStateInstance<TestGameState>();
				Game::GetInstance().SetCurrentGameState(newState);
			}
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			GFXRenderer->Clear(ClearFlags_Color, Color(24, 24, 24, 255), 1.0f, 0);
			GFXRenderer->SwapBuffers();
		}

		std::string_view GetStateName() const
		{
			return "Testing Game State Switching";
		}
	};

	struct Game::Impl
	{
		SDL_Window* GameWindow{};

		bool Running{};
		SDL_Event SDLEvent{};

		struct TimingData
		{
			u64 Ticks_Frequency = 0;

			u64 Ticks_LastFrame = 0;
			u64 Ticks_Current = 0;
			u64 Ticks_Delta = 0;

			f64 ActualFrameTime_Milliseconds = 0.0;
			u64 TargetFrameTime_Ticks = DefaultTargetFrameTime_Ticks;

			f64 DeltaTime_Milliseconds = 0.0;
		} Timing;

		struct GFXRendererData
		{
			RendererBackend Backend{};
			Renderer* Renderer = nullptr;
		} GFX;

		GameState* CurrentGameState = nullptr;

		bool Initialize()
		{
			LogMessage("--- Starshine %02d.%02d ---", 25, 8);

#if defined (_DEBUG)
			LogMessage("--- DEBUG BUILD ---");
#endif

			SDL_Init(SDL_INIT_EVERYTHING);

			u32 windowCreationFlags = 0;
			if (GFX.Backend == RendererBackend::OpenGL)
			{
				SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
				SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
				SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
				SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
				SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
				SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

				windowCreationFlags |= SDL_WINDOW_OPENGL;
			}

			GameWindow = SDL_CreateWindow("DIVA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, windowCreationFlags);

			Running = true;

			SDL_version sdlVersion{};
			SDL_GetVersion(&sdlVersion);
			const char* platformName = SDL_GetPlatform();

			LogInfo(LogName, "SDL Version: %d.%d.%d", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);
			LogInfo(LogName, "SDL Platform: %s", platformName);

			LogInfo(LogName, "Build Date: %s", BuildInfo::BuildDateString);
			LogInfo(LogName, "Git Information: %s, %s", BuildInfo::GitBranchName, BuildInfo::GitCommitHashString);

			Renderer::CreateInstance(GFX.Backend);
			GFX.Renderer = &Renderer::GetInstance();

			GFX.Renderer->Initialize(GameWindow);

			GameState* testState = GameStateHelpers::CreateGameStateInstance<TestGameState>();
			SetCurrentGameStateInstance(testState);

			return true;
		}

		void Destroy()
		{
			if (CurrentGameState != nullptr)
			{
				CurrentGameState->UnloadContent();
				CurrentGameState->Destroy();
				GameStateHelpers::DeleteGameStateInstance(CurrentGameState);
				LogInfo(LogName, "Current state destroyed");
			}

			SDL_DestroyWindow(GameWindow);
			SDL_Quit();
		}

		void UpdateTimingData()
		{
			if (Timing.Ticks_Frequency == 0)
			{
				Timing.Ticks_Frequency = SDL_GetPerformanceFrequency();
			}

			Timing.Ticks_LastFrame = Timing.Ticks_Current;
			Timing.Ticks_Current = SDL_GetPerformanceCounter() * 10000000 / Timing.Ticks_Frequency;
			Timing.Ticks_Delta = Timing.Ticks_Current - Timing.Ticks_LastFrame;

			Timing.ActualFrameTime_Milliseconds = static_cast<double>(Timing.Ticks_Delta) / 10000.0;

			while (Timing.Ticks_Delta < Timing.TargetFrameTime_Ticks)
			{
				SDL_Delay(1);
				Timing.Ticks_Current = SDL_GetPerformanceCounter() * 10000000 / Timing.Ticks_Frequency;
				Timing.Ticks_Delta = Timing.Ticks_Current - Timing.Ticks_LastFrame;
			}

			Timing.DeltaTime_Milliseconds = static_cast<double>(Timing.Ticks_Delta) / 10000.0;
		}

		void Loop()
		{
			while (Running)
			{
				UpdateTimingData();
				if (SDL_PollEvent(&SDLEvent) != 0)
				{
					switch (SDLEvent.type)
					{
					case SDL_QUIT:
						Running = false;
						break;
					}
				}

				if (CurrentGameState != nullptr)
				{
					CurrentGameState->Update(Timing.DeltaTime_Milliseconds);
				}

				if (CurrentGameState != nullptr)
				{
					CurrentGameState->Draw(Timing.DeltaTime_Milliseconds);
				}
			}

			Destroy();
		}

		bool SetCurrentGameStateInstance(GameState* stateInstance)
		{
			if (stateInstance == nullptr)
			{
				return false;
			}

			if (CurrentGameState != nullptr)
			{
				LogInfo(LogName, "Changing state: [%s] -> [%s]",
					CurrentGameState->GetStateName().data(), stateInstance->GetStateName().data());

				CurrentGameState->UnloadContent();
				CurrentGameState->Destroy();
				GameStateHelpers::DeleteGameStateInstance(CurrentGameState);
				LogInfo(LogName, "Previous state destroyed");
			}
			else
			{
				LogInfo(LogName, "Setting initial state: [%s]", stateInstance->GetStateName().data());
			}

			if (stateInstance->Initialize() != true)
			{
				return false;
			}
			LogInfo(LogName, "Initialized new state");

			if (stateInstance->LoadContent() != true)
			{
				return false;
			}
			LogInfo(LogName, "Loaded content of the new state");

			CurrentGameState = stateInstance;
		}
	};

	Game::Game() : impl(new Impl())
	{
	}

	void Game::CreateInstance()
	{
		if (GameInstance == nullptr)
		{
			GameInstance = new Game();
		}
	}

	void Game::DeleteInstance()
	{
		if (GameInstance != nullptr)
		{
			delete GameInstance;
		}
	}

	Game& Game::GetInstance()
	{
		assert(GameInstance != nullptr);
		return *GameInstance;
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

	f64 Game::GetDeltaTime_Milliseconds() const
	{
		return impl->Timing.DeltaTime_Milliseconds;
	}

	bool Game::SetCurrentGameState(GameState* stateInstance)
	{
		return impl->SetCurrentGameStateInstance(stateInstance);
	}
}
