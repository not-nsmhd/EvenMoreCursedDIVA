#pragma once
#include <string>
#include "../game.h"
#include "../gfx/lowlevel/backend.h"
#include "../gfx/sprite_renderer.h"
#include "../common/int_types.h"

namespace Testing
{
	class GFXBackendTest : public GameState
	{
	public:
		GFXBackendTest() {}

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();
	private:
		GFX::LowLevel::BlendState noBlend = {};
		mat4 projMatrix = {};

		GFX::SpriteRenderer spriteRenderer;
	};
}
