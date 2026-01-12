#pragma once
#include "game_state.h"

namespace Starshine::Testing
{
	class DataTest : public GameState
	{
	public:
		DataTest();
		~DataTest() = default;
	public:
		bool Initialize();
		bool LoadContent();

		void UnloadContent();
		void Destroy();

		void Update(f64 deltaTime_milliseconds);
		void Draw(f64 deltaTime_milliseconds);

		std::string_view GetStateName() const;

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
