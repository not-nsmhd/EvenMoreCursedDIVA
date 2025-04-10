#pragma once
#include <SDL2/SDL.h>

namespace Logging
{
	bool ConsoleExists();
	
	void LoggingInit();
	void LoggingQuit();

	void ToggleLoggingToConsole(bool enable);
	void ToggleLoggingToFile(bool enable);

	SDL_PRINTF_VARARG_FUNC(1) void LogMessage(const char* text, ...);
	SDL_PRINTF_VARARG_FUNC(2) void LogInfo(const char* component, const char* text, ...);
	SDL_PRINTF_VARARG_FUNC(2) void LogWarn(const char* component, const char* text, ...);
	SDL_PRINTF_VARARG_FUNC(2) void LogError(const char* component, const char* text, ...);
}
