#include "global_res.h"

namespace GlobalResources
{
	GFX::Font* DebugFont = nullptr;

	bool Load(GFX::LowLevel::Backend* backend)
	{
		DebugFont = new GFX::Font();
		DebugFont->LoadBMFont(backend, "fonts/debug.fnt");

		return true;
	}
	
	void Destroy()
	{
		if (DebugFont != nullptr)
		{
			DebugFont->Destroy();
			delete DebugFont;
		}
	}
}
