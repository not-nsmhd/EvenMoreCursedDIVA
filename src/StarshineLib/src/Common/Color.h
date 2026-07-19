#pragma once
#include "Types.h"
#include "MathExt.h"

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

		constexpr Color operator*(const f32& right)
		{
			u8 newR = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) * right, 0.0f, 255.0f));
			u8 newG = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(G) * right, 0.0f, 255.0f));
			u8 newB = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(B) * right, 0.0f, 255.0f));
			u8 newA = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(A) * right, 0.0f, 255.0f));
			return Color(newR, newG, newB, newA);
		}

		constexpr Color operator/(const f32& right)
		{
			if (right <= 0.0f)
				return Color(255, 255, 255, 255);

			u8 newR = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) / right, 0.0f, 255.0f));
			u8 newG = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(G) / right, 0.0f, 255.0f));
			u8 newB = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(B) / right, 0.0f, 255.0f));
			u8 newA = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(A) / right, 0.0f, 255.0f));
			return Color(newR, newG, newB, newA);
		}

		Color& operator+=(const Color& right)
		{
			R += right.R;
			G += right.G;
			B += right.B;
			A += right.A;
			return *this;
		}

		Color& operator-=(const Color& right)
		{
			R -= right.R;
			G -= right.G;
			B -= right.B;
			A -= right.A;
			return *this;
		}

		Color& operator*=(const f32& right)
		{
			R = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) * right, 0.0f, 255.0f));
			G = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) * right, 0.0f, 255.0f));
			B = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) * right, 0.0f, 255.0f));
			A = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) * right, 0.0f, 255.0f));
			return *this;
		}

		Color& operator/=(const f32& right)
		{
			if (right <= 0.0f)
			{
				R = 255;
				G = 255;
				B = 255;
				A = 255;
			}
			else
			{
				R = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) / right, 0.0f, 255.0f));
				G = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) / right, 0.0f, 255.0f));
				B = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) / right, 0.0f, 255.0f));
				A = static_cast<u8>(MathExtensions::Clamp<f32>(static_cast<f32>(R) / right, 0.0f, 255.0f));
			}
			return *this;
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
