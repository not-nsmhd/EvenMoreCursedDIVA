#pragma once
#include "Types.h"

namespace Starshine
{
	struct Rectangle
	{
		i32 X;
		i32 Y;
		i32 Width;
		i32 Height;

		Rectangle() = default;
		Rectangle(i32 width, i32 height) : X(0), Y(0), Width(width), Height(height) {}
		Rectangle(i32 x, i32 y, i32 width, i32 height) : X(x), Y(y), Width(width), Height(height) {}

		inline bool Contains(i32 x, i32 y) const 
		{ return (x >= X && x <= (X + Width) && y >= Y && y <= (Y + Height)); };

		inline bool Contains(i32 x, i32 y, i32 width, i32 height) const 
		{ return (x >= X && (x + width) <= (X + Width) && y >= Y && (y + height) <= (Y + Height)); };

		inline bool Contains(const ivec2& point) const { return Contains(point.x, point.y); };
		inline bool Contains(const Rectangle& rect) const { return Contains(rect.X, rect.Y, rect.Width, rect.Height); };

		inline bool Intersects(const Rectangle& rect) const
		{
			return Width > 0 && Height > 0 && rect.Width > 0 && rect.Height > 0 && 
				((rect.X + rect.Width) > X && rect.X < (X + Width) && 
				 (rect.Y + rect.Height) > Y && rect.Y < (Y + Height));
		};
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
