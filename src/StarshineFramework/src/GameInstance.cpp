#include <SDL2/SDL.h>
#include "Common/Logging/Logging.h"
#include "BuildInfo.h"
#include "GameInstance.h"
#include "Rendering/Device.h"
#include "Input/Keyboard.h"
#include "Input/Gamepad.h"

namespace Starshine
{
	using namespace Rendering;
	using namespace Input;

	struct GameInstance::Impl
	{
		GameInstance* Parent;
		SDL_Window* BaseWindow{};

		bool Running{ true };
		SDL_Event SDLEvent{};

		Device* GFXDevice{ nullptr };
		std::unique_ptr<GameState> CurrentState{};

		static constexpr u64 DefaultTargetFrameTime_Ticks = 166667;

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

		Impl(GameInstance* instance) : Parent(instance)
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
			Parent->GameWindow = std::make_unique<Window>("Even More Cursed DIVA", 1280, 720, SDL_WINDOW_OPENGL);

			if (!Parent->GameWindow->Exists())
			{
				LogMessage("Failed to create game window. Error: %s", SDL_GetError());
				SDL_Quit();
				return false;
			}

			BaseWindow = Parent->GameWindow->GetBaseWindow();

			Rendering::InitializeDevice(BaseWindow, DeviceType::OpenGL);
			GFXDevice = Rendering::GetDevice();

			Keyboard::Initialize();
			Gamepad::Initialize();

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

			Gamepad::Destroy();
			Keyboard::Destroy();
			Rendering::DestroyDevice();

			Parent->GameWindow = nullptr;
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
			Timing.Ticks_Current = SDL_GetPerformanceCounter() * 10000000 / Timing.Ticks_Frequency;
			Timing.Ticks_Delta = Timing.Ticks_Current - Timing.Ticks_LastFrame;

			Timing.Ticks_Error = Timing.Ticks_Delta - Timing.TargetFrameTime_Ticks;
			Timing.DeltaTime_Milliseconds = static_cast<double>(Timing.Ticks_Delta) / 10000.0;
		}

		void Loop()
		{
			while (Running)
			{
				UpdateTimingData();
				Keyboard::NextFrame();
				Gamepad::NextFrame();

				while (SDL_PollEvent(&SDLEvent))
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
					case SDL_CONTROLLERDEVICEADDED:
						Gamepad::Connect(SDLEvent.cdevice.which);
						break;
					case SDL_CONTROLLERDEVICEREMOVED:
						Gamepad::Disconnect();
						break;
					case SDL_CONTROLLERAXISMOTION:
					case SDL_CONTROLLERBUTTONDOWN:
					case SDL_CONTROLLERBUTTONUP:
					case SDL_CONTROLLERSENSORUPDATE:
						Gamepad::Poll();
						break;
					}
				}

				if (CurrentState != nullptr && !Timing.FirstFrame) { CurrentState->Update(Timing.DeltaTime_Milliseconds); }
				if (CurrentState != nullptr && !Timing.FirstFrame) { CurrentState->Draw(Timing.DeltaTime_Milliseconds); }
				else
				{
					GFXDevice->Clear(ClearFlags_Color, DefaultColors::Black, 1.0f, 0);
					GFXDevice->SwapBuffers();
				}

				Timing.FirstFrame = false;
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

			state->GameInstance = Parent;

			if (!state->Initialize()) { return false; }
			LogMessage("Initialized new state");

			if (!state->LoadContent()) { return false; }
			LogMessage("Loaded content of the new state");

			CurrentState = std::move(state);
			return true;
		}
	};

	GameInstance::GameInstance() : impl(std::make_unique<Impl>(this))
	{
	}

	GameInstance::~GameInstance()
	{
	}

	Window* const GameInstance::GetWindow()
	{
		if (GameWindow != nullptr) { return GameWindow.get(); }
		return nullptr;
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
