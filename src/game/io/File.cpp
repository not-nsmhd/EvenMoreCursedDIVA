#include "File.h"
#include <fstream>
#include <Windows.h>
#include "util/logging.h"

namespace Starshine::IO
{
	namespace File
	{
		using std::fstream;
		using std::ios;
		using std::string_view;
		using namespace Logging;

		constexpr const char* LogName = "Starshine::IO::File";

		constexpr const char PathSeparator = '/';
		constexpr const char PathSeparator_Windows = '\\';

		bool Exists(std::string_view filePath)
		{
#if defined (_WIN32)
			WIN32_FILE_ATTRIBUTE_DATA fileAttrib = {};

			if (GetFileAttributesExA(filePath.data(), GetFileExInfoStandard, &fileAttrib) == true)
			{
				return true;
			}
#endif
			return false;
		}

		size_t GetSize(std::string_view filePath)
		{
#if defined (_WIN32)
			WIN32_FILE_ATTRIBUTE_DATA fileAttrib = {};

			if (GetFileAttributesExA(filePath.data(), GetFileExInfoStandard, &fileAttrib) == true)
			{
#if defined (_WIN64)
				return (static_cast<size_t>(fileAttrib.nFileSizeHigh) << 32) | static_cast<size_t>(fileAttrib.nFileSizeLow);
#elif defined (_WIN32)
				return static_cast<size_t>(fileAttrib.nFileSizeLow);
#endif
			}
#endif
			return 0;
		}

		size_t ReadAllBytes(std::string_view filePath, u8** dest)
		{
			size_t fileSize = GetSize(filePath);

			fstream file = fstream(filePath.data(), ios::in | ios::binary);

			if (!file.good())
			{
				LogError(LogName, "Failed to open file \"%s\"", filePath.data());
				return 0;
			}

			u8* fileData = new u8[fileSize];
			file.read(reinterpret_cast<char*>(fileData), fileSize);
			file.close();

			*dest = fileData;
			return fileSize;
		}

		std::string_view GetParentDirectory(std::string_view path)
		{
			if (path.empty())
			{
				return string_view();
			}

			for (size_t i = path.size() - 1; i > 0; i--)
			{
				if (path.at(i) == PathSeparator || path.at(i) == PathSeparator_Windows)
				{
					return path.substr(0, i);
				}
			}
		}
	}
}
