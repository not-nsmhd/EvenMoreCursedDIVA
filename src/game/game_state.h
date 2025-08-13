#pragma once
#include "common/types.h"
#include <string_view>
#include <type_traits>

namespace Starshine
{
	class GameState : public NonCopyable
	{
	public:
		virtual bool Initialize() = 0;
		virtual bool LoadContent() = 0;

		virtual void UnloadContent() = 0;
		virtual void Destroy() = 0;

		virtual void Update(f64 deltaTime_milliseconds) = 0;
		virtual void Draw(f64 deltaTime_milliseconds) = 0;

		virtual std::string_view GetStateName() const = 0;
	};

	namespace GameStateHelpers
	{
		template <typename State>
		inline GameState* CreateGameStateInstance()
		{
			return new State();
		}

		inline void DeleteGameStateInstance(GameState* state)
		{
			delete state;
		}
	}
}
