#pragma once
#include <string>
#include "game.h"
#include "gfx/font.h"
#include "gfx/sprite_renderer.h"

namespace Testing
{
	class InputTest : public GameState
	{
	public:
		InputTest();

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();
	private:
		GFX::SpriteRenderer spriteRenderer;
		GFX::Font* debugFont;

		i32 scrollTest = 0;

		char keyboardTestString[1024] = {};
		char mouseTestString[1024] = {};
	};
}
