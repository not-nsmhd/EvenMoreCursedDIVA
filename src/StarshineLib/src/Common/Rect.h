#pragma once
#include "Types.h"

namespace Starshine
{
	struct Rectangle
	{
		i32 X{};
		i32 Y{};
		i32 Width{};
		i32 Height{};

		Rectangle() = default;
		constexpr Rectangle(i32 width, i32 height) : X(0), Y(0), Width(width), Height(height) {}
		constexpr Rectangle(i32 x, i32 y, i32 width, i32 height) : X(x), Y(y), Width(width), Height(height) {}

		inline bool Contains(i32 x, i32 y) const 
		{ return (x >= X && x <= (X + Width) && y >= Y && y <= (Y + Height)); };

		inline bool Contains(i32 x, i32 y, i32 width, i32 height) const 
		{ return (x >= X && (x + width) <= (X + Width) && y >= Y && (y + height) <= (Y + Height)); };

		inline bool Contains(const ivec2& point) const { return Contains(point.x, point.y); };
		inline bool Contains(const Rectangle& rect) const { return Contains(rect.X, rect.Y, rect.Width, rect.Height); };

		inline bool Intersects(i32 x, i32 y, i32 width, i32 height) const
		{
			return Width > 0 && Height > 0 && width > 0 && height > 0 &&
				((x + width) > X && x < (X + Width) &&
					(y + height) > Y && y < (Y + Height));
		};

		inline bool Intersects(const Rectangle& rect) const { return Intersects(rect.X, rect.Y, rect.Width, rect.Height); };

		constexpr i32 Area() const { return Width * Height; };

		constexpr ivec2 Position() const { return ivec2{ X, Y }; };
		constexpr ivec2 Size() const { return ivec2{ Width, Height }; };
		constexpr vec2 Center() const { return vec2{ Width / 2.0f, Height / 2.0f }; };
	};

	struct RectangleF
	{
		f32 X{};
		f32 Y{};
		f32 Width{};
		f32 Height{};

		RectangleF() = default;
		constexpr RectangleF(f32 width, f32 height) : X(0.0f), Y(0.0f), Width(width), Height(height) {}
		constexpr RectangleF(f32 x, f32 y, f32 width, f32 height) : X(x), Y(y), Width(width), Height(height) {}

		inline bool Contains(f32 x, f32 y) const
		{
			return (x >= X && x <= (X + Width) && y >= Y && y <= (Y + Height));
		};

		inline bool Contains(f32 x, f32 y, f32 width, f32 height) const
		{
			return (x >= X && (x + width) <= (X + Width) && y >= Y && (y + height) <= (Y + Height));
		};

		inline bool Contains(const vec2& point) const { return Contains(point.x, point.y); };
		inline bool Contains(const RectangleF& rect) const { return Contains(rect.X, rect.Y, rect.Width, rect.Height); };

		inline bool Intersects(f32 x, f32 y, f32 width, f32 height) const
		{
			return Width > 0 && Height > 0 && width > 0 && height > 0 &&
				((x + width) > X && x < (X + Width) &&
					(y + height) > Y && y < (Y + Height));
		};

		inline bool Intersects(const RectangleF& rect) const { return Intersects(rect.X, rect.Y, rect.Width, rect.Height); };

		constexpr f32 Area() const { return Width * Height; };

		constexpr vec2 Position() const { return vec2{ X, Y }; };
		constexpr vec2 Size() const { return vec2{ Width, Height }; };
		constexpr vec2 Center() const { return vec2{ Width / 2.0f, Height / 2.0f }; };
	};
};
