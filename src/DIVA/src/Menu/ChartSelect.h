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

		void Update(Starshine::GameTime& gameTime);
		void Draw(Starshine::GameTime& gameTime);

		inline static std::string_view GetStateName() { return "ChartSelect"; };

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};
	};
}
