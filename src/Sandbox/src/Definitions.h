#pragma once
#include "Common/Types.h"

namespace Sandbox
{
	enum GameStateID : i64
	{
		GameState_Invalid = -1,

		GameState_None = 0,

		GameState_AnimTest = 1,
		GameState_Editor = 2,

		GameState_FontTest = 3
	};
}
