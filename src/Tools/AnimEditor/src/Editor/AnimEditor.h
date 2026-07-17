#pragma once
#include <Common/Types.h>
#include <GameInstance.h>

namespace Starshine
{
	class AnimEditor : public GameState
	{
	public:
		AnimEditor();
		~AnimEditor();

	public:
		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void Update(Starshine::GameTime& gameTime);
		void Draw(Starshine::GameTime& gameTime);

		i64 GetStateID() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};
	};
}
