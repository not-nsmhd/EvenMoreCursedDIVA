#pragma once
#include "Common/Types.h"
#include <SDL2/SDL_endian.h>

namespace Starshine::IO
{
	enum class PointerSize : u8
	{
		Size32Bit,
		Size64Bit
	};

	enum class Endianess : u8
	{
		Little,
		Big,
		
		// HACK: F*ck it, let SDL decide it for us (Wii U port is in the works, don't worry)
		Native = (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? Little : Big
	};
}
