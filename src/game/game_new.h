#pragma once
#include "common/types.h"

namespace Starshine
{
	class Game : public NonCopyable
	{
	public:
		Game();
		~Game() = default;

	public:
		static void CreateInstance();
		static void DeleteInstance();

		static Game& GetInstance();

	public:
		// NOTE: Initialize and enter the main loop
		int Run();

	public:
		f64 GetDeltaTime_Milliseconds() const;
		
	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
