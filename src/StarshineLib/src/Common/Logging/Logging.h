#pragma once
#include <SDL2/SDL.h>

namespace Starshine
{
	bool ConsoleExists();

	SDL_PRINTF_VARARG_FUNC(1) void LogMessage(const char* text, ...);
	SDL_PRINTF_VARARG_FUNC(2) void LogInfo(const char* component, const char* text, ...);
	SDL_PRINTF_VARARG_FUNC(2) void LogWarn(const char* component, const char* text, ...);
	SDL_PRINTF_VARARG_FUNC(2) void LogError(const char* component, const char* text, ...);

	void SDLLogFunction(void* userdata, int category, SDL_LogPriority priority, const char* message);
}
