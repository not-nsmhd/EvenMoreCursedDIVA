#include "ChartSelect.h"
#include <Rendering/Render2D/SpriteRenderer.h>
#include "GameContext.h"
#include "IO/Path/Directory.h"
#include "input/Keyboard.h"
#include "MainGame/MainGame.h"
#include <vector>
#include "Formats/SongInfo.h"
#include "../Definitions.h"

namespace DIVA::Menu
{
	using namespace Starshine;
	using namespace Starshine::IO;
	using namespace Starshine::Graphics;
	using namespace Starshine::Rendering::Render2D;
	using namespace Starshine::Input;
	using namespace DIVA::Formats;

	enum class SubMenuID : i32
	{
		Main,
		ChartSelect,
		Options,
		Exit, /* NOTE: Technically not a menu, but it is a hacky way to
			   make sure that the "Exit" option is displayed in the main menu */

		Count
	};

	static constexpr std::array<std::string_view, EnumCount<SubMenuID>()> SubMenuNames
	{
		"Main Menu",
		"Chart Select",
		"Options",
		"Exit"
	};

	struct ChartSelect::Impl
	{
		Starshine::GameInstance* GameInstance{};

		SpriteRenderer* spriteRenderer{};
		Font* debugFont{};

		SubMenuID currentSubmenu{};

		i32 selectionIndex = 1;
		i32 currentDifficultyIndex{};

		Impl()
		{
		}

		~Impl()
		{
		}

		void Reset()
		{
		}

		void LoadContent()
		{
			spriteRenderer = GameContext::GetInstance()->SpriteRenderer.get();
			debugFont = GameContext::GetInstance()->DebugFont.get();
		}

		void Destroy()
		{
		}

		// NOTE: Convinience function that automatically resets the selection index
		void SetSubMenu(SubMenuID id)
		{
			selectionIndex = 0;
			currentDifficultyIndex = 0;
			currentSubmenu = id;

			if (currentSubmenu == SubMenuID::Main)
				selectionIndex = 1;
		}

		void UpdateMain()
		{
			if (Keyboard::IsKeyTapped(SDLK_DOWN))
			{
				selectionIndex++;
				if (selectionIndex >= EnumCount<SubMenuID>()) { selectionIndex = static_cast<i32>(SubMenuID::ChartSelect); }
			}

			if (Keyboard::IsKeyTapped(SDLK_UP))
			{
				selectionIndex--;
				if (selectionIndex <= 0) { selectionIndex = EnumCount<SubMenuID>() - 1; }
			}

			if (Keyboard::IsKeyTapped(SDLK_RETURN))
			{
				if (selectionIndex != static_cast<i32>(SubMenuID::Exit))
					SetSubMenu(static_cast<SubMenuID>(selectionIndex));
				else
					GameInstance->Quit();
			}
		}

		void UpdateChartSelect()
		{
			auto songList = GameContext::GetInstance()->SongList;

			if (Keyboard::IsKeyTapped(SDLK_DOWN))
			{
				selectionIndex++;
				if (selectionIndex >= songList.size()) { selectionIndex = 0; }
			}

			if (Keyboard::IsKeyTapped(SDLK_UP))
			{
				selectionIndex--;
				if (selectionIndex < 0) { selectionIndex = songList.size() - 1; }
			}

			if (Keyboard::IsKeyTapped(SDLK_RIGHT))
			{
				currentDifficultyIndex++;
				if (currentDifficultyIndex >= EnumCount<ChartDifficulty>()) { currentDifficultyIndex = 0; }
			}

			if (Keyboard::IsKeyTapped(SDLK_LEFT))
			{
				currentDifficultyIndex--;
				if (currentDifficultyIndex < 0) { currentDifficultyIndex = EnumCount<ChartDifficulty>() - 1; }
			}

			if (Keyboard::IsKeyTapped(SDLK_RETURN))
			{
				const SongInfo& info = songList[selectionIndex];

				if (!info.ChartFilePaths[currentDifficultyIndex].empty())
				{
					auto mgState = static_cast<MainGame::MainGameState*>(GetStatePointer(StateID::MainGame));
					mgState->LoadSettings.ChartPath = info.ChartFilePaths[currentDifficultyIndex];
					mgState->LoadSettings.LyricsPath = info.LyricsFilePath;
					mgState->LoadSettings.MusicPath = info.MusicFilePath;
					mgState->LoadSettings.SongName = info.Name;
					mgState->LoadSettings.Difficulty = static_cast<ChartDifficulty>(currentDifficultyIndex);
					GameInstance->SetState(mgState);
				}
			}

			if (Keyboard::IsKeyTapped(SDLK_ESCAPE))
				SetSubMenu(SubMenuID::Main);
		}

		void UpdateDisplayModeList()
		{
			auto window = GameInstance->GetWindow();
			auto dispModeList = window->GetDisplayModes();

			if (Keyboard::IsKeyTapped(SDLK_DOWN))
			{
				selectionIndex++;
				if (selectionIndex >= dispModeList.size() + 1) { selectionIndex = 0; }
			}

			if (Keyboard::IsKeyTapped(SDLK_UP))
			{
				selectionIndex--;
				if (selectionIndex < 0) { selectionIndex = dispModeList.size(); } // Last index is the return button
			}

			if (Keyboard::IsKeyTapped(SDLK_F11))
			{
				WindowMode mode = window->GetMode();
				window->SetMode(mode == WindowMode::Window ? WindowMode::Fullscreen : WindowMode::Window);
			}

			if (Keyboard::IsKeyTapped(SDLK_RETURN))
			{
				if (selectionIndex != dispModeList.size())
				{
					const SDL_DisplayMode& dispMode = dispModeList[selectionIndex];
					window->SetDisplayMode(dispMode);
				}
				else
					SetSubMenu(SubMenuID::Main);
			}
		}

		void Update()
		{
			switch (currentSubmenu)
			{
			case SubMenuID::Main:
				UpdateMain();
				break;
			case SubMenuID::ChartSelect:
				UpdateChartSelect();
				break;
			case SubMenuID::Options:
				UpdateDisplayModeList();
				break;
			case SubMenuID::Exit:
				// NOTE/HACK: A failsafe to make sure the menu still works correctly if I mess something up when switching submenus
				SetSubMenu(SubMenuID::Main);
				break;
			}
		}

		void DrawMain()
		{
			spriteRenderer->Font().PushString(debugFont, "Main Menu", vec2(16.0f, 16.0f), vec2(1.0f), DefaultColors::White);

			float yOffset = 0.0f;
			for (size_t i = 1; i < EnumCount<SubMenuID>(); i++)
			{
				const Color selectionBaseColor = i == selectionIndex ? DefaultColors::Yellow : DefaultColors::White;

				spriteRenderer->Font().PushString(debugFont, SubMenuNames[i], vec2(16.0f, 64.0f + yOffset), vec2(1.0f), selectionBaseColor);

				yOffset += debugFont->LineHeight;
			}
		}

		void DrawChartSelect()
		{
			auto songList = GameContext::GetInstance()->SongList;

			spriteRenderer->Font().PushString(debugFont, "Song Select", vec2(16.0f, 16.0f), vec2(1.0f), DefaultColors::White);

			static constexpr std::array<Color, EnumCount<ChartDifficulty>()> difficultyColors
			{
				Color { 0, 128, 255, 255 },
				Color { 64, 255, 64, 255 },
				Color { 255, 128, 0, 255 },
				Color { 255, 64, 64, 255 }
			};

			for (size_t i = 0; i < EnumCount<ChartDifficulty>(); i++)
			{
				spriteRenderer->Font().PushString(debugFont, ChartDifficultyNames[i],
					vec2(16.0f + (92.0f * i), 36.0f), vec2(1.0f), i == currentDifficultyIndex ? difficultyColors[i] : DefaultColors::White);
			}

			float yOffset = 0.0f;
			i32 curIndex = 0;
			for (auto& info : songList)
			{
				const Color selectionBaseColor = curIndex == selectionIndex ? DefaultColors::Yellow : DefaultColors::White;
				const u8 selectionAlpha = info.ChartFilePaths[currentDifficultyIndex].empty() ? 128 : 255;

				spriteRenderer->Font().PushString(debugFont, info.Name, vec2(16.0f, 64.0f + yOffset), vec2(1.0f),
					Color{ selectionBaseColor.R, selectionBaseColor.G, selectionBaseColor.B, selectionAlpha });

				yOffset += debugFont->LineHeight;
				curIndex++;
			}
		}

		void DrawDisplayModeList()
		{
			auto window = GameInstance->GetWindow();
			auto dispModeList = window->GetDisplayModes();

			spriteRenderer->Font().PushString(debugFont, "Options", vec2(16.0f, 16.0f), vec2(1.0f), DefaultColors::White);

			f32 yOffset = 0.0f;
			i32 curIndex = 0;

			char dispModeString[64]{};
			const SDL_DisplayMode& nativeDisplayMode = window->GetNativeDisplayMode();

			for (const auto& dispMode : dispModeList)
			{
				bool native = (dispMode.w == nativeDisplayMode.w && dispMode.h == nativeDisplayMode.h);
				const Color selectionBaseColor = curIndex == selectionIndex ? DefaultColors::Yellow : DefaultColors::White;

				SDL_snprintf(dispModeString, sizeof(dispModeString) - 1, "%dx%d (%dHz) %s",
					dispMode.w, dispMode.h, dispMode.refresh_rate, native ? "(native)" : "");

				spriteRenderer->Font().PushString(debugFont, dispModeString, vec2(16.0f, 64.0f + yOffset), vec2(1.0f), selectionBaseColor);

				yOffset += debugFont->LineHeight;
				curIndex++;
			}

			yOffset += debugFont->LineHeight;
			spriteRenderer->Font().PushString(debugFont, "Return", vec2(16.0f, 64.0f + yOffset), vec2(1.0f),
				curIndex == selectionIndex ? DefaultColors::Yellow : DefaultColors::White);

			const size_t modeIndex = static_cast<size_t>(window->GetMode());
			SDL_snprintf(dispModeString, sizeof(dispModeString) - 1, "Window Mode (F11): %s", WindowModeNames[modeIndex]);
			spriteRenderer->Font().PushString(debugFont, dispModeString, vec2(512.0f, 64.0f), vec2(1.0f), DefaultColors::White);
		}

		void Draw()
		{
			auto gfxDevice = spriteRenderer->GetRenderingDevice();

			gfxDevice->Clear(Rendering::ClearFlags_Color, DefaultColors::ClearColor_Menus, 1.0f, 0);
			spriteRenderer->SetBlendMode(BlendMode::Normal);

			switch (currentSubmenu)
			{
			case SubMenuID::Main:
				DrawMain();
				break;
			case SubMenuID::ChartSelect:
				DrawChartSelect();
				break;
			case SubMenuID::Options:
				DrawDisplayModeList();
				break;
			}

			spriteRenderer->RenderSprites(nullptr);
		}
	};

	ChartSelect::ChartSelect() : impl(std::make_unique<Impl>())
	{
	}

	ChartSelect::~ChartSelect()
	{
	}

	bool ChartSelect::Initialize()
	{
		impl->GameInstance = GameInstance;
		impl->Reset();
		return true;
	}

	bool ChartSelect::LoadContent()
	{
		impl->LoadContent();
		return true;
	}

	void ChartSelect::UnloadContent()
	{
	}

	void ChartSelect::Destroy()
	{
		impl->Destroy();
	}

	void ChartSelect::Update(GameTime& gameTime)
	{
		impl->Update();
	}

	void ChartSelect::Draw(GameTime& gameTime)
	{
		impl->Draw();
	}
}
