#pragma once
#include "Types.h"

namespace Starshine
{
	struct Color
	{
		u8 R{};
		u8 G{};
		u8 B{};
		u8 A{};

		constexpr Color() : R{}, G{}, B{}, A{} {};
		constexpr Color(u8 r, u8 g, u8 b) : R{ r }, G{ g }, B{ b }, A{ 255 } {};
		constexpr Color(u8 r, u8 g, u8 b, u8 a) : R{ r }, G{ g }, B{ b }, A{ a } {};
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
