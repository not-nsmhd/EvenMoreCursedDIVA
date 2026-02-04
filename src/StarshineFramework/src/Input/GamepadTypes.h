#pragma once
#include "Common/Types.h"

namespace Starshine::Input
{
	enum class GamepadButton : i32
	{
		Unknown = -1,
		Unbound = Unknown,

		Circle,
		Xbox_B = Circle,

		Cross,
		Xbox_A = Cross,

		Square,
		Xbox_X = Square,

		Triangle,
		Xbox_Y = Triangle,

		Options,
		Start = Options,

		Share,
		Select = Share,

		L1,
		LB = L1,

		L2,
		LT = L2,

		R1,
		RB = R1,

		R2,
		RT = R2,

		DPad_Left,
		DPad_Right,
		DPad_Up,
		DPad_Down,

		Count
	};

	enum class GamepadAxis : u32
	{
		LeftStick_X,
		LeftStick_Y,

		RightStick_X,
		RightStick_Y,

		L2,
		LT = L2,

		R2,
		RT = R2,

		Count
	};

	enum class GamepadStick : u32
	{
		Left,
		Right,
		Count
	};

	constexpr std::array<const char*, EnumCount<GamepadButton>()> GamepadButtonNames_PlayStation
	{
		"Circle",
		"Cross",
		"Square",
		"Triangle",

		"Options",
		"Share",

		"L1",
		"L2",
		"R1",
		"R2",

		"Left",
		"Right",
		"Up",
		"Down"
	};

	constexpr std::array<const char*, EnumCount<GamepadStick>()> GamepadStickNames
	{
		"Left",
		"Right"
	};
}
