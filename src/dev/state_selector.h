#pragma once
#include "game.h"
#include "gfx/sprite_renderer.h"

namespace Dev
{
	class StateSelector : public GameState
	{
	public:
		StateSelector();

		static StateSelector* GetInstance();

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();

	private:
		static StateSelector* instance;

		GFX::SpriteRenderer spriteRenderer;
	};
}
