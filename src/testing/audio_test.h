#pragma once
#include <string>
#include "../game.h"
#include "../gfx/font.h"
#include "../gfx/sprite_renderer.h"

namespace Testing
{
	class AudioTest : public GameState
	{
	public:
		AudioTest() {}

		static AudioTest* GetInstance();

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();

	private:
		static AudioTest* instance;

		GFX::SpriteRenderer spriteRenderer;
	};
}
