#pragma once
#include "GFX/Types.h"

namespace Starshine::Rendering
{
	enum class DeviceType : i32
	{
		OpenGL,
		D3D9, // TODO: Implement

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

	enum class VertexAttribType : u8
	{
		Position,
		Color,
		TexCoord
	};

	enum class VertexAttribFormat : u8
	{
		Byte,
		UnsignedByte,
		Short,
		UnsignedShort,
		Int,
		UnsignedInt,
		Float,

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
		"D3D9"
	};
}
