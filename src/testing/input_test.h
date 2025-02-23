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

		static InputTest* GetInstance();

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();

	private:
		static InputTest* instance;

		GFX::SpriteRenderer spriteRenderer;

		char keyboardTestString[1024] = {};
		char mouseTestString[1024] = {};
	};
}
