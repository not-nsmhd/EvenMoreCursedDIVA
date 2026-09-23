#pragma once
#include <Common/Types.h>
#include <GameInstance.h>

namespace DIVA::Sandbox
{
	class MFVideoTest : public Starshine::GameState
	{
	public:
		MFVideoTest();
		~MFVideoTest();

	public:
		bool Initialize();
		bool LoadContent();

		void UnloadContent();
		void Destroy();

		void Update(Starshine::GameTime& gameTime);
		void Draw(Starshine::GameTime& gameTime);

		inline std::string_view GetStateName() { return "MFVideoTest"; };

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};
	};
}
