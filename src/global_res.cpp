#include "global_res.h"

namespace GlobalResources
{
	GFX::Font* DebugFont;
	GFX::SpriteRenderer* SpriteRenderer;

	bool Load(GFX::LowLevel::Backend* backend)
	{
		DebugFont = new GFX::Font();
		DebugFont->LoadBMFont(backend, "fonts/debug.fnt");

		SpriteRenderer = new GFX::SpriteRenderer();
		SpriteRenderer->Initialize(backend);

		return true;
	}
	
	void Destroy()
	{
		if (DebugFont != nullptr)
		{
			DebugFont->Destroy();
			delete DebugFont;
		}

		if (SpriteRenderer != nullptr)
		{
			SpriteRenderer->Destroy();
			delete SpriteRenderer;
		}
	}
}
