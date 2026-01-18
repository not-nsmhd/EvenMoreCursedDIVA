#include "ChartSelect.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "IO/Path/Directory.h"
#include "input/Keyboard.h"
#include "MainGame/MainGame.h"
#include "game.h"
#include "build/build_info.h"
#include <vector>

namespace DIVA::Menu
{
	using namespace Starshine;
	using namespace Starshine::IO;
	using namespace Starshine::GFX;
	using namespace Starshine::GFX::Render2D;
	using namespace Starshine::Input;

	struct ChartSelect::Impl
	{
		SpriteRenderer* spriteRenderer{};
		Font debugFont;

		std::vector<std::string> chartPaths;

		i32 selectionIndex = 0;

		Impl()
		{
		}

		void LoadContent()
		{
			spriteRenderer = new SpriteRenderer();
			debugFont.ReadBMFont("diva/fonts/debug.fnt");
		}

		void ConstructChartList()
		{
			Directory::IterateFilesRecursive("diva/songdata", [&](std::string_view filePath)
				{
					chartPaths.emplace_back(filePath);
				});
		}

		void Destroy()
		{
			chartPaths.clear();
			debugFont.Destroy();
			spriteRenderer->Destroy();
			delete spriteRenderer;
		}

		void Update()
		{
			if (Keyboard::IsKeyTapped(SDLK_DOWN))
			{
				selectionIndex++;
				if (selectionIndex >= chartPaths.size()) { selectionIndex = 0; }
			}

			if (Keyboard::IsKeyTapped(SDLK_UP))
			{
				selectionIndex--;
				if (selectionIndex < 0) { selectionIndex = chartPaths.size() - 1; }
			}

			if (Keyboard::IsKeyTapped(SDLK_RETURN))
			{
				auto mgState = static_cast<MainGame::MainGameState*>(GameStateHelpers::CreateGameStateInstance<MainGame::MainGameState>());
				mgState->LoadSettings.ChartPath = chartPaths[selectionIndex];
				Game::GetInstance().SetCurrentGameState(mgState);
			}
		}

		void DrawBuildInfo()
		{
			float baseX = Renderer::GetInstance()->GetViewportSize().Width - 256.0f;

			char text[256] = {};
			int offset = SDL_snprintf(text, sizeof(text) - 1, "Even More Cursed DIVA\n");
			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "Starshine %02d.%02d\n", BuildInfo::BuildYear - 2000, BuildInfo::BuildMonth);

#if defined (_WIN32)
			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "Windows ");
#endif

#if defined (_M_AMD64)
			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "64-bit\n");
#endif

#if defined (_DEBUG)
			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "DEBUG BUILD\n");
#endif

			spriteRenderer->Font().PushString(debugFont, text, vec2(baseX, 16.0f), vec2(1.0f), DefaultColors::White);
		}

		void Draw()
		{
			Renderer::GetInstance()->Clear(ClearFlags_Color, DefaultColors::ClearColor_Menus, 1.0f, 0);
			spriteRenderer->SetBlendMode(BlendMode::Normal);

			spriteRenderer->Font().PushString(debugFont, "Chart Select", vec2(16.0f, 16.0f), vec2(1.0f), DefaultColors::White);

			float yOffset = 0.0f;
			i32 curIndex = 0;
			for (auto& path : chartPaths)
			{
				spriteRenderer->Font().PushString(debugFont, path, vec2(16.0f, 48.0f + yOffset), vec2(1.0f),
					curIndex == selectionIndex ? DefaultColors::Yellow : DefaultColors::White);

				yOffset += debugFont.LineHeight;
				curIndex++;
			}

			DrawBuildInfo();

			spriteRenderer->RenderSprites(nullptr);
			Renderer::GetInstance()->SwapBuffers();
		}
	};

	ChartSelect::ChartSelect() : impl(new Impl())
	{
	}

	bool ChartSelect::Initialize()
	{
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
		delete impl;
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
