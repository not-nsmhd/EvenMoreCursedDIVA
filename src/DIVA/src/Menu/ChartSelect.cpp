#include "ChartSelect.h"
#include <Rendering/Render2D/SpriteRenderer.h>
#include <BuildInfo.h>
#include "IO/Path/Directory.h"
#include "input/Keyboard.h"
#include "MainGame/MainGame.h"
#include <vector>
#include "Formats/SongInfo.h"

namespace DIVA::Menu
{
	using namespace Starshine;
	using namespace Starshine::IO;
	using namespace Starshine::GFX;
	using namespace Starshine::Rendering::Render2D;
	using namespace Starshine::Input;
	using namespace DIVA::Formats;

	struct ChartSelect::Impl
	{
		Starshine::GameInstance* GameInstance{};

		SpriteRenderer* spriteRenderer{};
		Font debugFont;

		std::vector<SongInfo> songList;

		i32 selectionIndex = 0;
		i32 currentDifficultyIndex{};

		Impl()
		{
		}

		~Impl()
		{
		}

		void LoadContent()
		{
			spriteRenderer = new SpriteRenderer();
			debugFont.ReadBMFont("diva/fonts/debug.fnt");
		}

		void ConstructChartList()
		{
#if 0
			Directory::IterateFilesRecursive("diva/songdata", [&](std::string_view filePath)
				{
					chartPaths.emplace_back(filePath);
				});
#endif
			Directory::IterateFiles("diva/songdata", [&](std::string_view filePath)
				{
					SongInfo info{};
					if (info.ParseFromFile(filePath))
					{
						songList.push_back(info);
					}
				});
		}

		void Destroy()
		{
			songList.clear();
			debugFont.Destroy();
			spriteRenderer->Destroy();
			delete spriteRenderer;
		}

		void Update()
		{
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
					auto mgState = std::make_unique<MainGame::MainGameState>();
					mgState->LoadSettings.ChartPath = info.ChartFilePaths[currentDifficultyIndex];
					mgState->LoadSettings.MusicPath = info.MusicFilePath;
					GameInstance->SetState(std::move(mgState));
				}
			}
		}

		void Draw()
		{
			auto gfxDevice = spriteRenderer->GetRenderingDevice();

			gfxDevice->Clear(Rendering::ClearFlags_Color, DefaultColors::ClearColor_Menus, 1.0f, 0);
			spriteRenderer->SetBlendMode(BlendMode::Normal);

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
				spriteRenderer->Font().PushString(debugFont, info.Name, vec2(16.0f, 64.0f + yOffset), vec2(1.0f),
					curIndex == selectionIndex ? DefaultColors::Yellow : DefaultColors::White);

				yOffset += debugFont.LineHeight;
				curIndex++;
			}

			spriteRenderer->RenderSprites(nullptr);
			gfxDevice->SwapBuffers();
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
		impl->ConstructChartList();
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

	void ChartSelect::Update(f64 deltaTime_ms)
	{
		impl->Update();
	}

	void ChartSelect::Draw(f64 deltaTime_ms)
	{
		impl->Draw();
	}

	std::string_view ChartSelect::GetStateName() const
	{
		return "Chart Select";
	}
}
