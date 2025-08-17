#pragma once
#include "common/types.h"

namespace Starshine::GFX
{
	enum class RendererBackendType : u32
	{
		OpenGL,
		D3D9,

		Count
	};

	constexpr std::array<std::string_view, EnumCount<RendererBackendType>()> RendererBackendTypeNames =
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

	enum class PrimitiveType : u8
	{
		Points,
		Lines,
		LineStrip,
		Triangles,
		TriangleStrip,

		Count
	};

	enum class IndexFormat : u8
	{
		Index16bit,
		Index32bit,

		Count
	};

	enum class BlendFactor : u8
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DestColor,
		OneMinusDestColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DestAlpha,
		OneMinusDestAlpha,

		Count
	};

	enum class BlendOperation : u8
	{
		Add,
		Subtract,
		SubtractInverse,
		Min,
		Max,

		Count
	};
}
