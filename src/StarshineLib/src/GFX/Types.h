#pragma once
#include "Common/Types.h"

namespace Starshine::GFX
{
	enum class TextureFormat : i32
	{
		Unknown = -1,

		RGBA8,
		RG8,
		A8,

		DXT1,
		DXT3,
		DXT5,

		Count
	};
}
