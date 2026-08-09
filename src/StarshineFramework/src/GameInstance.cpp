#include <SDL2/SDL.h>
#include "Common/Logging/Logging.h"
#include "BuildInfo.h"
#include "GameInstance.h"
#include "Rendering/Device.h"
#include "Input/Keyboard.h"
#include "Input/Gamepad.h"
#include "Audio/AudioEngine.h"

#include "ImGui/Core/imgui.h"
#include "ImGui/Core/backends/imgui_impl_sdl2.h"
#include "ImGui/Core/backends/imgui_impl_dx11.h"
#include "Rendering/D3D11/D3D11Device.h"
#include "ImGui/Core/imgui_styles.h"

namespace Starshine
{
	using namespace Rendering;
	using namespace Input;
	using namespace Audio;

	struct GameInstance::Impl
	{
		GameInstance* Parent;
		SDL_Window* BaseWindow{};

		bool ImGuiLoaded{ false };

		bool Running{ true };
		SDL_Event SDLEvent{};

		Device* GFXDevice{ nullptr };

		GameState* CurrentState{};
		i64 CurrentStateID{};

		static constexpr TimeSpan TargetFrameTime = TimeSpan(16667);

		struct TimingData
		{
			bool FirstFrame = true;
			u64 Ticks_Frequency = 0;

			u64 Ticks_LastFrame = 0;
			u64 Ticks_Current = 0;
			u64 Ticks_Delta = 0;
			u64 Ticks_Error = 0;

			GameTime GameTime;
		} Timing;

		Impl(GameInstance* instance) : Parent(instance)
		{
		}
		
		~Impl()
		{
		}

		bool Initialize(bool initImGui)
		{
#if defined (_DEBUG)
			LogMessage("--- Starshine %02d.%02d [Debug] ---", BuildInfo::BuildYear - 2000, BuildInfo::BuildMonth);
#endif

			LogMessage("SDL Platform: %s", SDL_GetPlatform());

			LogMessage("Build Date: %s", BuildInfo::BuildDateString);
			LogMessage("Git Information: %s, %s", BuildInfo::GitBranchName, BuildInfo::GitCommitHashString);

			SDL_Init(SDL_INIT_EVERYTHING);
			Parent->GameWindow = std::make_unique<Window>("", 1280, 720, SDL_WINDOW_SHOWN);

			if (!Parent->GameWindow->Exists())
			{
				LogMessage("Failed to create game window. Error: %s", SDL_GetError());
				SDL_Quit();
				return false;
			}

			BaseWindow = Parent->GameWindow->GetBaseWindow();

			Rendering::InitializeDevice(BaseWindow, DeviceType::D3D11);
			GFXDevice = Rendering::GetDevice();

			Keyboard::Initialize();
			Gamepad::Initialize();

			AudioEngine::CreateInstance();

			if (initImGui)
			{
				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO();
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
				io.ConfigWindowsMoveFromTitleBarOnly = true;

				ImGui_ImplSDL2_InitForD3D(Parent->GameWindow->GetBaseWindow());

				// HACK: no Starshine::Rendering wrapper yet...
				if (Rendering::GetDeviceType() == Rendering::DeviceType::D3D11)
				{
					Rendering::D3D11::D3D11Device* gfxDevice = static_cast<Rendering::D3D11::D3D11Device*>(Rendering::GetDevice());
					ID3D11Device* d3dDevice = gfxDevice->GetBaseDevice();
					ID3D11DeviceContext* d3dDevContext = nullptr;
					d3dDevice->GetImmediateContext(&d3dDevContext);
					ImGui_ImplDX11_Init(d3dDevice, d3dDevContext);
				}

				ImGuiStyle& style = ImGui::GetStyle();
				style.FontSizeBase = 18.0f;
				io.Fonts->AddFontFromFileTTF("imgui/fonts/SourceSans3-Regular.ttf");
				ImGuiStyles::ApplyImGuiStyle();

				ImGuiLoaded = true;
			}

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

			if (ImGuiLoaded)
			{
				ImGui_ImplDX11_Shutdown();
				ImGui_ImplSDL2_Shutdown();
				ImGui::DestroyContext();
			}

			if (Rendering::GetDevice() != nullptr)
				Rendering::GetDevice()->ReportExistingObjects();

			AudioEngine::DestroyInstance();

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
			Timing.Ticks_Current = SDL_GetPerformanceCounter() * 1000000 / Timing.Ticks_Frequency;
			Timing.Ticks_Delta = Timing.Ticks_Current - Timing.Ticks_LastFrame;

			if (Timing.Ticks_Error > TargetFrameTime.Microseconds)
			{
				Timing.Ticks_Error = 0;
			}

			while (Timing.Ticks_Delta < TargetFrameTime.Microseconds - Timing.Ticks_Error)
			{
				SDL_Delay(1);
				Timing.Ticks_Current = SDL_GetPerformanceCounter() * 1000000 / Timing.Ticks_Frequency;
				Timing.Ticks_Delta = Timing.Ticks_Current - Timing.Ticks_LastFrame;
			}
			Timing.Ticks_Current = SDL_GetPerformanceCounter() * 1000000 / Timing.Ticks_Frequency;
			Timing.Ticks_Delta = Timing.Ticks_Current - Timing.Ticks_LastFrame;

			Timing.Ticks_Error = Timing.Ticks_Delta - TargetFrameTime.Microseconds;

			Timing.GameTime.ElapsedFrameTime.Microseconds = Timing.Ticks_Delta;
			Timing.GameTime.TimeSinceLaunch.Microseconds += Timing.Ticks_Delta;
			Timing.GameTime.TargetFrameTime = TargetFrameTime;
		}

		void Loop()
		{
			// HACK: This is to make sure that the viewport is set to the correct size if the game resizes the window before entering the loop
			ivec2 windowSize = Parent->GameWindow->GetSize();
			GFXDevice->OnWindowResize(windowSize.x, windowSize.y);

			while (Running)
			{
				UpdateTimingData();
				Keyboard::NextFrame();
				Gamepad::NextFrame();

				while (SDL_PollEvent(&SDLEvent))
				{
					if (ImGuiLoaded)
					{
						ImGui_ImplSDL2_ProcessEvent(&SDLEvent);
					}

					switch (SDLEvent.type)
					{
					case SDL_QUIT:
						Running = false;
						break;
					case SDL_WINDOWEVENT:
						switch (SDLEvent.window.event)
						{
						case SDL_WINDOWEVENT_CLOSE:
							Running = false;
							break;
						case SDL_WINDOWEVENT_RESIZED:
							GFXDevice->OnWindowResize(SDLEvent.window.data1, SDLEvent.window.data2);
							break;
						}
						break;
					case SDL_KEYDOWN:
					case SDL_KEYUP:
						if (ImGuiLoaded)
						{
							if (!ImGui::GetIO().WantCaptureKeyboard)
								Keyboard::Poll(SDLEvent.key);
						}
						else
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

				if (ImGuiLoaded)
				{
					ImGui_ImplSDL2_NewFrame();
					ImGui_ImplDX11_NewFrame();
					ImGui::NewFrame();
				}

				if (CurrentState != nullptr && !Timing.FirstFrame) { CurrentState->Update(Timing.GameTime); }
				if (CurrentState != nullptr && !Timing.FirstFrame) { CurrentState->Draw(Timing.GameTime); }
				else
				{
					GFXDevice->Clear(ClearFlags_Color, DefaultColors::Black, 1.0f, 0);
				}

				if (ImGuiLoaded)
				{
					ImGui::Render();
					ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
				}

				GFXDevice->SwapBuffers();
				Timing.FirstFrame = false;
			}
		}

		bool SetState(GameState* state)
		{
			if (state == nullptr) { return false; }

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

			CurrentState = state;
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

	bool GameInstance::Initialize(bool initImGui)
	{
		return impl->Initialize(initImGui);
	}

	void GameInstance::EnterLoop()
	{
		impl->Loop();
	}

	void GameInstance::Destroy()
	{
		impl->Destroy();
	}

	void GameInstance::Quit()
	{
		impl->Running = false;
	}

	bool GameInstance::SetState(GameState* state)
	{
		return impl->SetState(state);
	}
}
