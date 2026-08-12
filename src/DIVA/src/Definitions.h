#pragma once
#include "Common/Types.h"
#include <GameInstance.h>

#include "Menu/ChartSelect.h"
#include "MainGame/MainGame.h"

namespace DIVA
{
	enum class StateID : i32
	{
		NotSet = -1,

		ChartSelect,
		MainGame,

		Count
	};

	static std::unique_ptr<Starshine::GameState> StateInstances[Starshine::EnumCount<StateID>()]
	{
		std::make_unique<Menu::ChartSelect>(),
		std::make_unique<MainGame::MainGameState>()
	};

	inline Starshine::GameState* GetStatePointer(StateID id)
	{
		if (id == StateID::NotSet)
			return nullptr;

		return StateInstances[static_cast<size_t>(id)].get();
	}

	constexpr vec2 BaseResolution = vec2(1920.0f, 1080.0f);
}
