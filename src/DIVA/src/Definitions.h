#pragma once
#include "Common/Types.h"

namespace DIVA
{
	enum GameStateID : i64
	{
		GameState_Invalid = -1,

		GameState_None = 0,

		GameState_ChartSelect = 1,
		GameState_MainGame = 2
	};
}
