#pragma once
#include "Common/Types.h"
#include <GameInstance.h>

namespace Sandbox::VideoTest
{
	class VideoTestState : public Starshine::GameState
	{
	public:
		VideoTestState();
		~VideoTestState();

	public:
		bool Initialize();
		bool LoadContent();

		void UnloadContent();
		void Destroy();

		void Update(Starshine::GameTime& gameTime);
		void Draw(Starshine::GameTime& gameTime);

		inline std::string_view GetStateName() { return "VideoTest"; };

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};
	};
}
