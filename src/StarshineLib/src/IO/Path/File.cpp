#include "File.h"
#include <fstream>
#include <Windows.h>

namespace Starshine::IO
{
	namespace File
	{
		using std::fstream;
		using std::ios;
		using std::string_view;

		constexpr const char PathSeparator = '/';
		constexpr const char PathSeparator_Windows = '\\';

		bool Exists(std::string_view filePath)
		{
#if defined (_WIN32)
			WIN32_FILE_ATTRIBUTE_DATA fileAttrib = {};

			if (GetFileAttributesExA(filePath.data(), GetFileExInfoStandard, &fileAttrib) == TRUE)
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

			if (GetFileAttributesExA(filePath.data(), GetFileExInfoStandard, &fileAttrib) == TRUE)
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

		FileStream OpenRead(std::string_view filePath)
		{
			FileStream result;
			result.OpenRead(filePath);
			return result;
		}

		FileStream CreateWrite(std::string_view filePath)
		{
			FileStream result;
			result.CreateWrite(filePath);
			return result;
		}

		size_t ReadAllBytes(std::string_view filePath, std::unique_ptr<u8[]>& destData)
		{
			FileStream fileStream = OpenRead(filePath);
			if (!fileStream.IsReadable() || !fileStream.IsOpen())
			{
				fileStream.Close();
				return 0; 
			}

			size_t fileSize = 0;
			destData = std::make_unique<u8[]>(fileStream.GetSize());

			fileStream.ReadBuffer(destData.get(), fileSize);

			fileStream.Close();
			return fileSize;
		}

		bool WriteAllBytes(std::string_view filePath, const void* data, size_t size)
		{
			if (data == nullptr || size == 0) { return false; }

			FileStream fileStream = CreateWrite(filePath);
			if (!fileStream.IsWritable() || !fileStream.IsOpen())
			{
				fileStream.Close();
				return 0;
			}

			fileStream.WriteBuffer(data, size);

			fileStream.Close();
			return true;
		}

		size_t ReadAllBytes(std::string_view filePath, u8** dest)
		{
			size_t fileSize = GetSize(filePath);

			fstream file = fstream(filePath.data(), ios::in | ios::binary);

			if (!file.good())
			{
				return 0;
			}

			u8* fileData = new u8[fileSize];
			file.read(reinterpret_cast<char*>(fileData), fileSize);
			file.close();

			*dest = fileData;
			return fileSize;
		}
	}
}
