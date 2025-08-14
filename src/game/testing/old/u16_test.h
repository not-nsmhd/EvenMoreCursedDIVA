#pragma once
#include <string>
#include "../game.h"
#include "../gfx/font.h"

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
		u8* testTextBuffer = nullptr;
		size_t testTextBufferSize = 0;

		GFX::Font font;
		GFX::LowLevel::BlendState alphaBlend = {};
	};
}
