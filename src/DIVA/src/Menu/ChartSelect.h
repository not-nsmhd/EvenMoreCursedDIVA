#pragma once
#include "Common/Types.h"
#include <GameInstance.h>

namespace DIVA::Menu
{
	class ChartSelect : public Starshine::GameState
	{
	public:
		ChartSelect();
		~ChartSelect();

	public:
		bool Initialize();
		bool LoadContent();

		void UnloadContent();
		void Destroy();

		void Update(f64 deltaTime_ms);
		void Draw(f64 deltaTime_ms);

		std::string_view GetStateName() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};
	};
}
