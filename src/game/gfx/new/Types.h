#pragma once
#include "common/types.h"

namespace Starshine::GFX
{
	enum class RendererBackendType : u32
	{
		OpenGL,
		D3D9, // TODO: Implement

		Count
	};

	constexpr const char* RendererBackendNames[EnumCount<RendererBackendType>()] =
	{
		"OpenGL",
		"D3D9"
	};

	enum ClearFlags : u8
	{
		ClearFlags_Color = 1 << 0,
		ClearFlags_Depth = 1 << 1,
		ClearFlags_Stencil = 1 << 2
	};
}
