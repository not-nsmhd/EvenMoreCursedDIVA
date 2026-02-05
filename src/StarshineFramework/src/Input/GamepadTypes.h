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

	enum class GamepadStickDirection : u32
	{
		None,
		
		Left,
		Right,
		Up,
		Down,

		Count
	};

	enum GamepadStickDirectionFlags : u32
	{
		GamepadStickDirection_None = 0,
		GamepadStickDirection_Left = (1 << 0),
		GamepadStickDirection_Right = (1 << 1),
		GamepadStickDirection_Up = (1 << 2),
		GamepadStickDirection_Down = (1 << 3),

		GamepadStickDirection_Any = (GamepadStickDirection_Left | GamepadStickDirection_Right | GamepadStickDirection_Up | GamepadStickDirection_Down)
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
