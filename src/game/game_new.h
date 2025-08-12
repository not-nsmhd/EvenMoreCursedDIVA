#pragma once
#include "common/types.h"

namespace Starshine
{
	class Game : NonCopyable
	{
	public:
		Game();
		~Game() = default;

		// NOTE: Initialize and enter the main loop
		int Run();
	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
