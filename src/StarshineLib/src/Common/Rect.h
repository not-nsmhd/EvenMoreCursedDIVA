#pragma once
#include "Types.h"

namespace Starshine
{
	struct Rectangle
	{
		u32 X;
		u32 Y;
		u32 Width;
		u32 Height;

		Rectangle() = default;
		Rectangle(u32 width, u32 height) : X(0), Y(0), Width(width), Height(height) {}
		Rectangle(u32 x, u32 y, u32 width, u32 height) : X(x), Y(y), Width(width), Height(height) {}
	};

	struct RectangleF
	{
		float X;
		float Y;
		float Width;
		float Height;

		RectangleF() = default;
		RectangleF(float width, float height) : X(0.0f), Y(0.0f), Width(width), Height(height) {}
		RectangleF(float x, float y, float width, float height) : X(x), Y(y), Width(width), Height(height) {}
	};
};
