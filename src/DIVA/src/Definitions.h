#pragma once
#include "Common/Types.h"
#include <GameInstance.h>

#include "Menu/ChartSelect.h"
#include "MainGame/MainGame.h"
#include "Sandbox/RNGTest.h"
#include "Sandbox/AsyncTest.h"

namespace DIVA
{
	enum class StateID : i32
	{
		NotSet = -1,

		ChartSelect,
		MainGame,

		Sandbox_RNGTest,
		Sandbox_AsyncTest,

		Count
	};

	static std::unique_ptr<Starshine::GameState> StateInstances[Starshine::EnumCount<StateID>()]
	{
		std::make_unique<Menu::ChartSelect>(),
		std::make_unique<MainGame::MainGameState>(),

		std::make_unique<Sandbox::RNGTest>(),
		std::make_unique<Sandbox::AsyncTest>(),
	};

	inline Starshine::GameState* GetStatePointer(StateID id)
	{
		if (id == StateID::NotSet)
			return nullptr;

		return StateInstances[static_cast<size_t>(id)].get();
	}

	constexpr vec2 BaseResolution = vec2(1920.0f, 1080.0f);
}
