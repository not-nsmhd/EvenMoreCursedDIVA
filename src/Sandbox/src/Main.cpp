#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GFX/SpritePacker.h"
#include "Rendering/Device.h"
#include "Rendering/Render2D/SpriteSheet.h"
#include "Rendering/Render2D/SpriteRenderer.h"
#include "Input/Keyboard.h"

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

		sprRenderer.SpriteSheet().PushSprite(spriteSheet, 0, sprPos, vec2(1.0f), DefaultColors::White);
		sprRenderer.SpriteSheet().PushSprite(spriteSheet, 7, vec2(780.0f, 360.0f), vec2(1.0f), DefaultColors::White);

		char text[64] = {};
		sprintf_s(text, sizeof(text) - 1, "Elapsed Time: %.3f\nDelta Time: %.3f", elapsedTime, deltaTime_milliseconds);

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
