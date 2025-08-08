#pragma once
#include "gfx/font.h"
#include "gfx/sprite_renderer.h"

namespace GlobalResources
{
	extern GFX::Font* DebugFont;
	extern GFX::SpriteRenderer* SpriteRenderer;

	bool Load(GFX::LowLevel::Backend* backend);
	void Destroy();
}
