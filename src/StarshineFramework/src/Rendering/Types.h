#pragma once
#include "GFX/Types.h"

namespace Starshine::Rendering
{
	enum class DeviceType : i32
	{
		OpenGL, // TODO: Implement
		D3D11,

		Count
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

	enum class BlendFactor : i8
	{
		None = -1,

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

	enum class BlendOperation : i8
	{
		None = -1,

		Add,
		Subtract,
		SubtractInverse,
		Min,
		Max,

		Count
	};

	enum class PolygonOrientation : u8
	{
		Clockwise,
		CounterClockwise,

		Count
	};

	enum class Face : u8
	{
		Front,
		Back,
		FrontAndBack,

		Count
	};

	constexpr std::array<const char*, EnumCount<DeviceType>()> DeviceTypeNames =
	{
		"OpenGL",
		"D3D11"
	};
}
