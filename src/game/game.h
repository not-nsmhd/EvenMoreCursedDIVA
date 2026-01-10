#pragma once
#include <Common/Types.h>
#include "game_state.h"

namespace Starshine
{
	class Game : public NonCopyable
	{
	public:
		Game();
		~Game() = default;

	public:
		static void CreateInstance();
		static void DeleteInstance();

		static Game& GetInstance();

	public:
		// NOTE: Initialize and enter the main loop
		int Run();

	public:
		f64 GetDeltaTime_Milliseconds() const;

	public:
		// NOTE: This assumes you have an existing game state instance where you haven't called any of the base functions
		bool SetCurrentGameState(GameState* stateInstance);
		
	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
