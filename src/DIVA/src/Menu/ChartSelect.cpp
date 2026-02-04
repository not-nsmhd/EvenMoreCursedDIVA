#include "ChartSelect.h"
#include <Rendering/Render2D/SpriteRenderer.h>
#include <BuildInfo.h>
#include "IO/Path/Directory.h"
#include "input/Keyboard.h"
#include "MainGame/MainGame.h"
#include <vector>

namespace DIVA::Menu
{
	using namespace Starshine;
	using namespace Starshine::IO;
	using namespace Starshine::GFX;
	using namespace Starshine::Rendering::Render2D;
	using namespace Starshine::Input;

	struct ChartSelect::Impl
	{
		Starshine::GameInstance* GameInstance{};

		SpriteRenderer* spriteRenderer{};
		Font debugFont;

		std::vector<std::string> chartPaths;

		i32 selectionIndex = 0;

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
				auto mgState = std::make_unique<MainGame::MainGameState>();
				mgState->LoadSettings.ChartPath = chartPaths[selectionIndex];
				GameInstance->SetState(std::move(mgState));
			}
		}

		void DrawBuildInfo()
		{
			auto gfxDevice = spriteRenderer->GetRenderingDevice();
			float baseX = gfxDevice->GetViewportSize().Width - 256.0f;

			char text[256] = {};
			int offset = SDL_snprintf(text, sizeof(text) - 1, "Even More Cursed DIVA\n");
			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "Starshine %02d.%02d\n", BuildInfo::BuildYear - 2000, BuildInfo::BuildMonth);

#if defined (_WIN32)
			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "Windows ");
#endif

#if defined (__X86_64__) || defined (_M_X64)
			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "64-bit");
#endif

#if defined (_DEBUG)
			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "\nDEBUG BUILD");
#endif

			spriteRenderer->Font().PushString(debugFont, text, vec2(baseX, 16.0f), vec2(1.0f), DefaultColors::White);
		}

		void Draw()
		{
			auto gfxDevice = spriteRenderer->GetRenderingDevice();

			gfxDevice->Clear(Rendering::ClearFlags_Color, DefaultColors::ClearColor_Menus, 1.0f, 0);
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
