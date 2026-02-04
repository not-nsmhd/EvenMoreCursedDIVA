#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GFX/SpritePacker.h"
#include "Rendering/Device.h"
#include "Rendering/Render2D/SpriteSheet.h"
#include "Rendering/Render2D/SpriteRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Gamepad.h"
#include "Common/MathExt.h"

using namespace Starshine;
using namespace Starshine::GFX;
using namespace Starshine::Rendering;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine::Input;

class TestState : public GameState
{
public:
	TestState() {};

public:
	bool Initialize()
	{
		GFXDevice = Rendering::GetDevice();
		return true;
	}

	bool LoadContent()
	{
		SpritePacker sprPacker;
		sprPacker.AddFromDirectory("diva/sprites/devtest");
		sprPacker.Pack();

		spriteSheet.CreateFromSpritePacker(sprPacker);
		sprPacker.Clear();

		testFont.ReadBMFont("diva/fonts/debug.fnt");

		return true;
	}

	void UnloadContent()
	{
		spriteSheet.Destroy();
		testFont.Destroy();
		sprRenderer.Destroy();
	}

	void Destroy()
	{
	}

	void Update(f64 deltaTime_milliseconds)
	{
		elapsedTime += deltaTime_milliseconds;
		
		if (Keyboard::IsKeyDown(SDLK_RIGHT)) { sprPos.x += 1.0f; }
		if (Keyboard::IsKeyDown(SDLK_LEFT)) { sprPos.x -= 1.0f; }
		if (Keyboard::IsKeyDown(SDLK_DOWN)) { sprPos.y += 1.0f; }
		if (Keyboard::IsKeyDown(SDLK_UP)) { sprPos.y -= 1.0f; }
	}

	void Draw(f64 deltaTime_milliseconds)
	{
		GFXDevice->Clear(ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);

		static constexpr vec2 axisBoxPosition = vec2(640.0f, 360.0f);
		static constexpr vec2 axisBoxOrigin = vec2(64.0f);

		sprRenderer.PushOutlineRect(axisBoxPosition - axisBoxOrigin, vec2(128.0f), vec2(0.0f), DefaultColors::White);
		sprRenderer.PushLine(vec2(axisBoxPosition.x - axisBoxOrigin.x, axisBoxPosition.y), 0.0f, 128.0f, DefaultColors::White);
		sprRenderer.PushLine(vec2(axisBoxPosition.x, axisBoxPosition.y - axisBoxOrigin.y), MathExtensions::PiOver2, 128.0f, DefaultColors::White);

		vec2 leftStick = { Gamepad::GetAxis(GamepadAxis::LeftStick_X), Gamepad::GetAxis(GamepadAxis::LeftStick_Y) };
		sprRenderer.SetSpritePosition((leftStick * 64.0f) + axisBoxPosition);
		sprRenderer.SetSpriteSize(vec2(8.0f));
		sprRenderer.SetSpriteOrigin(vec2(4.0f));
		sprRenderer.SetSpriteColor(DefaultColors::Red);
		sprRenderer.PushSprite(nullptr);

		char text[512] = {};
		int offset = SDL_snprintf(text, sizeof(text) - 1, "Elapsed Time: %.3f\nDelta Time: %.3f\n", elapsedTime, deltaTime_milliseconds);

		if (Gamepad::IsConnected())
		{
			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "Gamepad Buttons: ");
			for (size_t i = 0; i < EnumCount<GamepadButton>(); i++)
			{
				if (Gamepad::IsButtonDown(static_cast<GamepadButton>(i)))
				{
					offset += SDL_snprintf(text + offset, sizeof(text) - 1, "%s ",
						GamepadButtonNames_PlayStation[i]);
				}
			}

			/*offset += SDL_snprintf(text + offset, sizeof(text) - 1, "\nGamepad Sticks (Held): ");
			for (size_t i = 0; i < EnumCount<GamepadStick>(); i++)
			{
				if (Gamepad::IsStickHeld(static_cast<GamepadStick>(i)))
				{
					offset += SDL_snprintf(text + offset, sizeof(text) - 1, "%s ",
						GamepadStickNames[i]);
				}
			}*/

			offset += SDL_snprintf(text + offset, sizeof(text) - 1, "\nGamepad Sticks (Pulled): ");
			for (size_t i = 0; i < EnumCount<GamepadStick>(); i++)
			{
				if (Gamepad::IsStickPulled(static_cast<GamepadStick>(i)))
				{
					offset += SDL_snprintf(text + offset, sizeof(text) - 1, "%s ",
						GamepadStickNames[i]);
				}
			}
		}

		sprRenderer.Font().PushString(testFont, text, vec2(0.0f, 0.0f), vec2(1.0f, 1.0f), DefaultColors::White);

		sprRenderer.RenderSprites(nullptr);

		GFXDevice->SwapBuffers();
	}

	std::string_view GetStateName() const { return "Test State"; }

private:
	f64 elapsedTime{};

	Rendering::Device* GFXDevice{};
	SpriteRenderer sprRenderer;

	SpriteSheet spriteSheet;
	Font testFont;

	vec2 sprPos{ 640.0f, 360.0f };
};

int SDL_main(int argc, char* argv[])
{
	GameInstance game;
	
	if (game.Initialize())
	{
		game.SetState(std::make_unique<TestState>());
		game.EnterLoop();
		return 0;
	}

	return 1;
}
