#pragma once
#include "Common/Types.h"
#include <memory>

namespace Starshine
{
	class GameInstance
	{
	public:
		GameInstance();
		~GameInstance();

	public:
		bool Initialize();
		void EnterLoop();

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{ nullptr };
	};
}
