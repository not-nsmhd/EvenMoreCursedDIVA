#pragma once
#include <string>
#include "../game.h"
#include "../gfx/font.h"
#include "../gfx/sprite_renderer.h"

namespace Testing
{
	class U16Test : public GameState
	{
	public:
		U16Test() {}

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();
	private:
		std::u16string testText;

		GFX::Font font;
		GFX::SpriteRenderer spriteRenderer;

		GFX::LowLevel::BlendState alphaBlend = {};
	};
}
