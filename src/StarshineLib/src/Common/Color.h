#pragma once
#include "Types.h"

namespace Starshine
{
	static constexpr f32 ColorNormFactor{ 1.0f / 255.0f };

	struct Color
	{
		u8 R{};
		u8 G{};
		u8 B{};
		u8 A{};

		constexpr Color() : R{}, G{}, B{}, A{} {};
		constexpr Color(u8 r, u8 g, u8 b) : R{ r }, G{ g }, B{ b }, A{ 255 } {};
		constexpr Color(u8 r, u8 g, u8 b, u8 a) : R{ r }, G{ g }, B{ b }, A{ a } {};

		constexpr Color(const vec4& vector) : 
			R{ static_cast<u8>(vector.r * 255.0f) },
			G{ static_cast<u8>(vector.g * 255.0f) },
			B{ static_cast<u8>(vector.b * 255.0f) },
			A{ static_cast<u8>(vector.a * 255.0f) } {};

		constexpr Color operator+(const Color& right)
		{ 
			return Color(R + right.R, G + right.G, B + right.B, A + right.A);
		}

		constexpr Color operator-(const Color& right)
		{
			return Color(R - right.R, G - right.G, B - right.B, A - right.A);
		}

		Color& operator+=(const Color& right)
		{
			R += right.R;
			G += right.G;
			B += right.B;
			A += right.A;
		}

		Color& operator-=(const Color& right)
		{
			R -= right.R;
			G -= right.G;
			B -= right.B;
			A -= right.A;
		}

		constexpr vec4 ToVector4()
		{
			return vec4(
				static_cast<f32>(R) * ColorNormFactor,
				static_cast<f32>(G) * ColorNormFactor,
				static_cast<f32>(B) * ColorNormFactor,
				static_cast<f32>(A) * ColorNormFactor);
		}
	};

	namespace DefaultColors
	{
		constexpr Color Transparent { 0, 0, 0, 0 };

		constexpr Color Black { 0, 0, 0, 255 };
		constexpr Color White { 255, 255, 255, 255 };
		constexpr Color Gray { 128, 128, 128, 255 };

		constexpr Color Red { 255, 0, 0, 255 };
		constexpr Color Green { 0, 255, 0, 255 };
		constexpr Color Blue { 0, 0, 255, 255 };

		constexpr Color Yellow { 255, 255, 0, 255 };
		constexpr Color Cyan { 0, 255, 255, 255 };
		constexpr Color Purple { 255, 0, 255, 255 };

		constexpr Color ClearColor_Menus { 24, 24, 24, 255 };
		constexpr Color ClearColor_InGame { 0, 24, 24, 255 };
	};
};
