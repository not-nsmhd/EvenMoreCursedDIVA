#pragma once
#include "Common/Types.h"
#include <GameInstance.h>

#include "Editor/AnimEditor.h"

namespace Starshine
{
	enum class StateID : i32
	{
		NotSet = -1,

		AnimEditor,

		Count
	};

	static std::unique_ptr<Starshine::GameState> StateInstances[Starshine::EnumCount<StateID>()]
	{
		std::make_unique<AnimEditor>(),
	};

	inline Starshine::GameState* GetStatePointer(StateID id)
	{
		if (id == StateID::NotSet)
			return nullptr;

		return StateInstances[static_cast<size_t>(id)].get();
	}

	constexpr vec2 BaseResolution = vec2(1920.0f, 1080.0f);
}
