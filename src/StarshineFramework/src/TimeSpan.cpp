#include "TimeSpan.h"
#include <SDL2/SDL_timer.h>

namespace Starshine
{
	struct TimingData
	{
		i64 PerformanceFrequency{ 0 };
		i64 StartCounter{ 0 };
	} GlobalTimingData;

	TimeSpan TimeSpan::GetTimeNow()
	{
		if (GlobalTimingData.PerformanceFrequency == 0)
		{
			GlobalTimingData.PerformanceFrequency = SDL_GetPerformanceFrequency();
			GlobalTimingData.StartCounter = SDL_GetPerformanceCounter();
		}

		return TimeSpan(SDL_GetPerformanceCounter() * 1000000 / GlobalTimingData.PerformanceFrequency);
	}
}
