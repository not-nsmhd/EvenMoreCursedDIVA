#include "Directory.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Starshine::IO
{
	namespace Directory
	{
		bool Exists(std::string_view directoryPath)
		{
#ifdef _WIN32
			WIN32_FILE_ATTRIBUTE_DATA dirAttribs{};
			BOOL result = GetFileAttributesExA(directoryPath.data(), GetFileExInfoStandard, &dirAttribs);
			return (result == TRUE) && (dirAttribs.dwFileAttributes != INVALID_FILE_ATTRIBUTES) && (dirAttribs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
			return false;
#endif
		}
	}
}
