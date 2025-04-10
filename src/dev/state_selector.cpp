#include "state_selector.h"
#include "../gfx/font.h"
#include "../global_res.h"
#include <string>

using std::string;

namespace Dev
{
	static GFX::Font* debugFont;
	static int selectedStateIndex = 0;

	static char debugInfoText[1024] = {};

	StateSelector::StateSelector()
	{
	}
	
	bool StateSelector::Initialize()
	{
		selectedStateIndex = 0;
		SDL_memset(debugInfoText, 0, sizeof(debugInfoText));
		debugFont = GlobalResources::DebugFont;
		spriteRenderer.Initialize(graphicsBackend);
		return true;
	}
	
	bool StateSelector::LoadContent()
	{
		return true;
	}
	
	void StateSelector::UnloadContent()
	{	
	}
	
	void StateSelector::Destroy()
	{
		spriteRenderer.Destroy();	
	}
	
	void StateSelector::OnResize(u32 newWidth, u32 newHeight)
	{	
	}
	
	void StateSelector::Update()
	{
		if (keyboardState->IsKeyTapped(SDL_SCANCODE_DOWN))
		{
			if (selectedStateIndex < static_cast<int>(GameStates::DEVSTATE_STATE_SELECTOR) - 1)
			{
				selectedStateIndex++;
			}
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_UP))
		{
			if (selectedStateIndex > 0)
			{
				selectedStateIndex--;
			}
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_RETURN))
		{
			game->SetState(static_cast<GameStates>(selectedStateIndex));
		}

		if (keyboardState->IsKeyTapped(SDL_SCANCODE_ESCAPE))
		{
			game->Quit();
		}

		// --------

		size_t pos = 0;

		int buildYear = 0;
		int buildMonth = 0;
		game->GetVersionNumber(&buildYear, &buildMonth);

		const char* osName = game->GetPlatformName();
#if defined (_WIN32)
		const char* bitWidth = "32-bit";
#elif defined (_WIN64)
		const char* bitWidth = "64-bit";
#endif

#ifdef _DEBUG
		pos += SDL_snprintf(debugInfoText + pos, 1023, "DEBUG BUILD\n");
#endif
		pos += SDL_snprintf(debugInfoText + pos, 1023, "Starshine %02d.%02d\n", buildYear, buildMonth);
		pos += SDL_snprintf(debugInfoText + pos, 1023, "%s %s\n", osName, bitWidth);
	}
	
	void StateSelector::Draw()
	{
		graphicsBackend->Clear(GFX::LowLevel::ClearFlags::GFX_CLEAR_COLOR, Common::Color(24, 24, 24), 1.0f, 0);
		graphicsBackend->SetBlendState(&GFX::LowLevel::DefaultBlendStates::AlphaBlend);

		debugFont->PushString(spriteRenderer, "State Selector", glm::vec2(16.0f, 16.0f), glm::vec2(1.0f), Common::DefaultColors::White);

		const char* stateName = nullptr;
		float textOffset_y = 64.0f;
		for (int i = 0; i < static_cast<int>(GameStates::DEVSTATE_STATE_SELECTOR); i++)
		{
			stateName = GameStateNames[i];
			debugFont->PushString(spriteRenderer, stateName, 64, glm::vec2(64.0f, textOffset_y), glm::vec2(1.0f), Common::DefaultColors::White);

			if (i == selectedStateIndex)
			{
				debugFont->PushString(spriteRenderer, ">>", glm::vec2(32.0f, textOffset_y), glm::vec2(1.0f), Common::DefaultColors::White);
			}

			textOffset_y += debugFont->LineHeight;
		}

		debugFont->PushString(spriteRenderer, debugInfoText, glm::vec2(16.0f, game->windowHeight - 128.0f), glm::vec2(1.0f), Common::DefaultColors::White);

		spriteRenderer.RenderSprites(nullptr);	

		graphicsBackend->SwapBuffers();
	}
}
