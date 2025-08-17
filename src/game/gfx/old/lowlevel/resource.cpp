#include "resource.h"
#include <SDL2/SDL_stdinc.h>

namespace GFX::LowLevel
{
	const char* Resource::GetDebugName()
	{
		if (!nameSet)
		{
			return nullptr;
		}
		return name;
	}
	
	void Resource::SetDebugName(const char* name)
	{
		size_t newLen = SDL_utf8strnlen(name, 127);
		size_t finalLen = SDL_min(127, newLen);

		SDL_memcpy(this->name, name, finalLen);
		this->name[finalLen] = '\0';
		nameSet = true;
	}
}
