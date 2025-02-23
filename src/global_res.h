#pragma once
#include "gfx/font.h"

namespace GlobalResources
{
	extern GFX::Font* DebugFont;

	bool Load(GFX::LowLevel::Backend* backend);
	void Destroy();
}
