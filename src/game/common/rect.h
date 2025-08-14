#pragma once
#include "types.h"

namespace Common
{
	struct Rectangle
	{
		u32 x;
		u32 y;
		u32 width;
		u32 height;

		Rectangle() = default;
		Rectangle(u32 width, u32 height) : x(0), y(0), width(width), height(height) {}
		Rectangle(u32 x, u32 y, u32 width, u32 height) : x(x), y(y), width(width), height(height) {}
	};

	struct RectangleF
	{
		float x;
		float y;
		float width;
		float height;

		RectangleF() = default;
		RectangleF(float width, float height) : x(0.0f), y(0.0f), width(width), height(height) {}
		RectangleF(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}
	};
};
