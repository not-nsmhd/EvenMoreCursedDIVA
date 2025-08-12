#pragma once
#include "game.h"

namespace Testing
{
	class AudioTest : public GameState
	{
	public:
		AudioTest();
		~AudioTest();

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();

	private:
		struct StateInternal;
		StateInternal* stateInternal{ nullptr };
	};
}
