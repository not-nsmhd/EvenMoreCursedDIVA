#include "Directory.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Starshine::IO
{
	namespace Directory
	{
		bool Exists(std::string_view dirPath)
		{
#ifdef _WIN32
			WIN32_FILE_ATTRIBUTE_DATA dirAttribs{};
			BOOL result = GetFileAttributesExA(dirPath.data(), GetFileExInfoStandard, &dirAttribs);
			return (result == TRUE) && (dirAttribs.dwFileAttributes != INVALID_FILE_ATTRIBUTES) && (dirAttribs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
			return false;
#endif
		}

		std::string GetFullPath(std::string_view dirPath)
		{
#ifdef _WIN32
			CHAR pathBuffer[MAX_PATH]{};
			DWORD length = GetFullPathNameA(dirPath.data(), MAX_PATH, pathBuffer, NULL);
			if (length > 0)
				return std::string(pathBuffer, static_cast<size_t>(length));
#endif // _WIN32
			return "";
		}
	}
}
