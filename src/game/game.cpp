#include "game.h"
#include "build/build_info.h"
#include "util/logging.h"
#include <SDL2/SDL.h>
#include "gfx/Renderer.h"
#include "audio/AudioEngine.h"
#include "input/Keyboard.h"
#include "testing/AudioTest.h"
#include "testing/RenderingTest.h"
#include "main_game/MainGame.h"

namespace Starshine
{
	using namespace Logging;
	using namespace GFX;
	using namespace Audio;
	using namespace Input;

	using Common::Color;

	constexpr const char* LogName = "Starshine::Game";
	constexpr u64 DefaultTargetFrameTime_Ticks = 166667;

	Game* GameInstance = nullptr;

	struct Game::Impl
	{
		SDL_Window* GameWindow{};

		bool Running{};
		SDL_Event SDLEvent{};

		struct TimingData
		{
			bool FirstFrame = true;
			u64 Ticks_Frequency = 0;

			u64 Ticks_LastFrame = 0;
			u64 Ticks_Current = 0;
			u64 Ticks_Delta = 0;
			u64 Ticks_Error = 0;

			f64 ActualFrameTime_Milliseconds = 0.0;
			u64 TargetFrameTime_Ticks = DefaultTargetFrameTime_Ticks;

			f64 DeltaTime_Milliseconds = 0.0;
		} Timing;

		struct GFXRendererData
		{
			RendererBackendType BackendType{};
			Renderer* Renderer = nullptr;
		} GFX;

		GameState* CurrentGameState = nullptr;

		bool Initialize()
		{
			tm buildDate = {};
			SDL_sscanf(BuildInfo::BuildDateString, "%04d.%02d.%02dT%02d:%02d%02d",
				&buildDate.tm_year, &buildDate.tm_mon, &buildDate.tm_mday, &buildDate.tm_hour, &buildDate.tm_min, &buildDate.tm_sec);

			LogMessage("--- Starshine %02d.%02d ---", buildDate.tm_year - 2000, buildDate.tm_mon);

#if defined (_DEBUG)
			LogMessage("--- DEBUG BUILD ---");
#endif
			SDL_Init(SDL_INIT_EVERYTHING);

			SDL_version sdlVersion{};
			SDL_GetVersion(&sdlVersion);
			const char* platformName = SDL_GetPlatform();

			LogInfo(LogName, "SDL Version: %d.%d.%d", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);
			LogInfo(LogName, "SDL Platform: %s", platformName);

			LogInfo(LogName, "Build Date: %s", BuildInfo::BuildDateString);
			LogInfo(LogName, "Git Information: %s, %s", BuildInfo::GitBranchName, BuildInfo::GitCommitHashString);

			u32 windowCreationFlags = 0;
			if (GFX.BackendType == RendererBackendType::OpenGL)
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

			Renderer::CreateInstance(GFX.BackendType);
			GFX.Renderer = Renderer::GetInstance();
			GFX.Renderer->Initialize(GameWindow);

			AudioEngine::CreateInstance();
			AudioEngine::GetInstance()->Initialize();

			Keyboard::Initialize();

			GameState* mainGameState = GameStateHelpers::CreateGameStateInstance<Testing::AudioTest>();
			SetCurrentGameStateInstance(mainGameState);

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

			AudioEngine::GetInstance()->Destroy();
			Keyboard::Destroy();

			GFX.Renderer->Destroy();
			Renderer::DeleteInstance();
			GFX.Renderer = nullptr;

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

			if (Timing.Ticks_Error > Timing.TargetFrameTime_Ticks)
			{
				Timing.Ticks_Error = 0;
			}

			while (Timing.Ticks_Delta < Timing.TargetFrameTime_Ticks - Timing.Ticks_Error)
			{
				SDL_Delay(1);
				Timing.Ticks_Current = SDL_GetPerformanceCounter() * 10000000 / Timing.Ticks_Frequency;
				Timing.Ticks_Delta = Timing.Ticks_Current - Timing.Ticks_LastFrame;
			}

			Timing.Ticks_Error = Timing.Ticks_Delta - Timing.TargetFrameTime_Ticks;
			Timing.DeltaTime_Milliseconds = static_cast<double>(Timing.Ticks_Delta) / 10000.0;
		}

		void Loop()
		{
			while (Running)
			{
				UpdateTimingData();

				if (Timing.FirstFrame)
				{
					Timing.FirstFrame = false;
					continue;
				}

				Keyboard::NextFrame();

				if (SDL_PollEvent(&SDLEvent) != 0)
				{
					switch (SDLEvent.type)
					{
					case SDL_QUIT:
						Running = false;
						break;
					case SDL_KEYDOWN:
					case SDL_KEYUP:
						Keyboard::Poll(SDLEvent.key);
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
			return true;
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
