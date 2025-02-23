#include "input_test.h"
#include "common/math_ext.h"
#include "global_res.h"

#include <fstream>

using namespace Common;
using std::fstream;
using std::ios;

namespace Testing
{
	static GFX::Font* debugFont;

	static i32 scrollTest = 0;

	InputTest::InputTest()
	{
	}

	InputTest* InputTest::instance = new InputTest();
	
	InputTest* InputTest::GetInstance()
	{
		return instance;
	}
	
	bool InputTest::Initialize()
	{
		return true;
	}
	
	bool InputTest::LoadContent()
	{
		debugFont = GlobalResources::DebugFont;
		spriteRenderer.Initialize(graphicsBackend);
		return true;
	}
	
	void InputTest::UnloadContent()
	{
		spriteRenderer.Destroy();
	}
	
	void InputTest::Destroy()
	{
	}
	
	void InputTest::OnResize(u32 newWidth, u32 newHeight)
	{
	}
	
	void InputTest::Update()
	{
		SDL_memset(keyboardTestString, 0, sizeof(keyboardTestString));
		SDL_memset(mouseTestString, 0, sizeof(mouseTestString));

		int pos = 0;
		pos += SDL_snprintf(keyboardTestString + pos, sizeof(keyboardTestString) - 1, "Keyboard (Space):\n");
		pos += SDL_snprintf(keyboardTestString + pos, sizeof(keyboardTestString) - 1, "IsDown: %u\n", keyboardState->IsKeyDown(SDL_SCANCODE_SPACE));
		pos += SDL_snprintf(keyboardTestString + pos, sizeof(keyboardTestString) - 1, "IsUp: %u\n", keyboardState->IsKeyUp(SDL_SCANCODE_SPACE));
		pos += SDL_snprintf(keyboardTestString + pos, sizeof(keyboardTestString) - 1, "IsTapped: %u\n", keyboardState->IsKeyTapped(SDL_SCANCODE_SPACE));
		pos += SDL_snprintf(keyboardTestString + pos, sizeof(keyboardTestString) - 1, "IsReleased: %u\n\n", keyboardState->IsKeyReleased(SDL_SCANCODE_SPACE));

		glm::ivec2 curPos = mouseState->GetCurrentPosition();
		glm::ivec2 prevPos = mouseState->GetPreviousPosition();
		glm::ivec2 relPos = mouseState->GetRelativePosition();

		pos = 0;
		pos += SDL_snprintf(mouseTestString + pos, sizeof(mouseTestString) - 1, "Mouse:\n");
		pos += SDL_snprintf(mouseTestString + pos, sizeof(mouseTestString) - 1, "Current: {X: %d, Y: %d}\n", curPos.x, curPos.y);
		pos += SDL_snprintf(mouseTestString + pos, sizeof(mouseTestString) - 1, "Previous: {X: %d, Y: %d}\n", prevPos.x, prevPos.y);
		pos += SDL_snprintf(mouseTestString + pos, sizeof(mouseTestString) - 1, "Relative: {X: %d, Y: %d}\n", relPos.x, relPos.y);

		pos += SDL_snprintf(mouseTestString + pos, sizeof(mouseTestString) - 1, "Left Button: %u\n", mouseState->IsMouseButtonTapped(Input::MouseButton::MOUSE_LEFT));
		pos += SDL_snprintf(mouseTestString + pos, sizeof(mouseTestString) - 1, "Right Button: %u\n", mouseState->IsMouseButtonTapped(Input::MouseButton::MOUSE_RIGHT));
		pos += SDL_snprintf(mouseTestString + pos, sizeof(mouseTestString) - 1, "Middle Button: %u\n", mouseState->IsMouseButtonTapped(Input::MouseButton::MOUSE_MIDDLE));

		glm::ivec2 scrollValue = mouseState->GetScrollWheelValue();

		if (scrollValue.y > 0) // down
		{
			scrollTest++;
		}
		else if (scrollValue.y < 0) // up
		{
			scrollTest--;
		}

		pos += SDL_snprintf(mouseTestString + pos, sizeof(mouseTestString) - 1, "Scroll Value: {X: %d, Y: %d}\n", scrollValue);
		pos += SDL_snprintf(mouseTestString + pos, sizeof(mouseTestString) - 1, "Scroll Test: %04d\n", scrollTest);
	}
	
	void InputTest::Draw()
	{
		graphicsBackend->Clear(GFX::LowLevel::ClearFlags::GFX_CLEAR_COLOR, Common::Color(0, 24, 24, 255), 1.0f, 0);
		graphicsBackend->SetBlendState(&GFX::LowLevel::DefaultBlendStates::AlphaBlend);

		debugFont->PushString(spriteRenderer, keyboardTestString, sizeof(keyboardTestString) - 1, vec2(4.0f, 4.0f), vec2(1.0f), Common::DefaultColors::White);
		debugFont->PushString(spriteRenderer, mouseTestString, sizeof(mouseTestString) - 1, vec2(256.0f, 4.0f), vec2(1.0f), Common::DefaultColors::White);
		spriteRenderer.RenderSprites(nullptr);

		graphicsBackend->SwapBuffers();
	}
}
