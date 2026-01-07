#include "logging.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <chrono>
#include <fstream>

using std::fstream;
using std::ios;

namespace Logging
{
	static bool consoleExists = false;
	static fstream logFile;

	static bool logToConsole = false;
	static bool logToFile = false;

	static char tempBuf[1024] = {};

	bool ConsoleExists()
	{
		return consoleExists;
	}
	
	void LoggingInit()
	{
		if (logToFile)
		{
			std::time_t timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			logFile.open("gamelog.txt", ios::out | ios::binary);

			if (logFile.bad())
			{
				logToFile = false;
			}
		}

#ifdef _WIN32
		consoleExists = (GetConsoleWindow() != NULL);
#endif
	}
	
	void LoggingQuit()
	{
		if (logToFile)
		{
			logFile.flush();
			logFile.close();
		}
	}
	
	void ToggleLoggingToConsole(bool enable)
	{
		logToConsole = enable;
	}
	
	void ToggleLoggingToFile(bool enable)
	{
		logToFile = enable;
	}
	
	void LogMessage(const char* text, ...)
	{
		va_list list;

		va_start(list, text);
		int written = SDL_vsnprintf(tempBuf, 1023, text, list);
		
		if (logToFile)
		{
			logFile.write(tempBuf, written);
			logFile << '\n';
		}

		if (consoleExists && logToConsole)
		{
			vprintf(text, list);
			putchar('\n');
		}

		va_end(list);
	}
	
	void LogInfo(const char* component, const char* text, ...)
	{
		logFile << "[";
		logFile << component;
		logFile << "][INFO] ";

		va_list list;

		va_start(list, text);
		int written = SDL_vsnprintf(tempBuf, 1023, text, list);
		
		if(logToFile)
		{
			logFile.write(tempBuf, written);
			logFile << '\n';
		}

		if (consoleExists && logToConsole)
		{
			printf("[%s][INFO] ", component);
			vprintf(text, list);
			putchar('\n');
		}

		va_end(list);
	}
	
	void LogWarn(const char* component, const char* text, ...)
	{
		logFile << "[";
		logFile << component;
		logFile << "][WARN] ";

		va_list list;

		va_start(list, text);
		int written = SDL_vsnprintf(tempBuf, 1023, text, list);
		
		if (logToFile)
		{
			logFile.write(tempBuf, written);
			logFile << '\n';
		}

		if (consoleExists && logToConsole)
		{
			printf("[%s][WARN] ", component);
			vprintf(text, list);
			putchar('\n');
		}

		va_end(list);
	}
	
	void LogError(const char* component, const char* text, ...)
	{
		logFile << "[";
		logFile << component;
		logFile << "][ERROR] ";

		va_list list;

		va_start(list, text);
		int written = SDL_vsnprintf(tempBuf, 1023, text, list);
		
		if (logToFile)
		{
			logFile.write(tempBuf, written);
			logFile << '\n';
		}

		if (consoleExists && logToConsole)
		{
			printf("[%s][ERROR] ", component);
			vprintf(text, list);
			putchar('\n');
		}

		va_end(list);
	}

	void SDLLogFunction(void* userdata, int category, SDL_LogPriority priority, const char* message)
	{
		switch (priority)
		{
		case SDL_LogPriority::SDL_LOG_PRIORITY_INFO:
		case SDL_LogPriority::SDL_LOG_PRIORITY_DEBUG:
		case SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE:
			LogInfo("SDL", message);
			break;
		case SDL_LogPriority::SDL_LOG_PRIORITY_WARN:
			LogWarn("SDL", message);
			break;
		case SDL_LogPriority::SDL_LOG_PRIORITY_CRITICAL:
		case SDL_LogPriority::SDL_LOG_PRIORITY_ERROR:
			LogError("SDL", message);
			break;
		}
	}
}
