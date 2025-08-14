#pragma once
#include "types.h"

namespace Common
{
	struct Color
	{
		u8 R;
		u8 G;
		u8 B;
		u8 A;

		constexpr Color() : R{}, G{}, B{}, A{} {};
		constexpr Color(u8 r, u8 g, u8 b) : R{ r }, G{ g }, B{ b }, A{ 255 } {};
		constexpr Color(u8 r, u8 g, u8 b, u8 a) : R{ r }, G{ g }, B{ b }, A{ a } {};
	};

	struct HSVColor
	{
		float Hue;
		float Saturation;
		float Value;
		float Alpha;

		HSVColor() = default;
		HSVColor(float h, float s, float v) { Hue = h; Saturation = s; Value = v; Alpha = 1.0f; };
		HSVColor(float h, float s, float v, float a) { Hue = h; Saturation = s; Value = v; Alpha = a; };
	};

	struct Color HSVtoRGBA(HSVColor hsv);

	namespace DefaultColors
	{
		const Color Transparent		= { 0,		0,		0,		0 };

		const Color Black			= { 0,		0,		0,		255 };
		const Color White			= { 255,	255,	255,	255 };
		const Color Gray			= { 128,	128,	128,	255 };

		const Color Red				= { 255,	0,		0,		255 };
		const Color Green			= { 0,		255,	0,		255 };
		const Color Blue			= { 0,		0,		255,	255 };

		const Color Yellow			= { 255,	255,	0,		255 };
		const Color Cyan			= { 0,		255,	255,	255 };
		const Color Purple			= { 255,	0,		255,	255 };
	};
};
