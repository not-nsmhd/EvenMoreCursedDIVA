#pragma once
#include "Common/Types.h"
#include <GameInstance.h>
#include <type_traits>

#include "VideoTest/VideoTestState.h"

namespace Sandbox
{
	enum class StateID : i32
	{
		NotSet = -1,

		VideoTest,

		Count
	};

	static std::unique_ptr<Starshine::GameState> StateInstances[Starshine::EnumCount<StateID>()]
	{
		std::make_unique<VideoTest::VideoTestState>()
	};

	template <typename StateType>
	StateType* GetStatePointer(StateID id)
	{
		if (id == StateID::NotSet)
			return nullptr;

		return static_cast<StateType*>(StateInstances[static_cast<size_t>(id)].get());
	}
}
