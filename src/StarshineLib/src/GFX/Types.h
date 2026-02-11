#pragma once
#include "Common/Types.h"

namespace Starshine::GFX
{
	enum class TextureFormat : i32
	{
		Unknown = -1,

		RGBA8,
		RG8,
		R8,

		DXT1,
		DXT3,
		DXT5,

		Count
	};

	constexpr std::array<size_t, EnumCount<GFX::TextureFormat>()> TexturePixelSizes
	{
		4,
		2,
		1,

		// TODO: Implement
		0,
		0,
		0
	};
}
