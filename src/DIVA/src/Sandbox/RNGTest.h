#pragma once
#include <Common/Types.h>
#include <GameInstance.h>

namespace DIVA::Sandbox
{
	class RNGTest : public Starshine::GameState
	{
	public:
		RNGTest();
		~RNGTest();

	public:
		bool Initialize();
		bool LoadContent();

		void UnloadContent();
		void Destroy();

		void Update(Starshine::GameTime& gameTime);
		void Draw(Starshine::GameTime& gameTime);

		inline std::string_view GetStateName() { return "RNGTest"; };

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};
	};
}
