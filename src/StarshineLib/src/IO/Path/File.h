#pragma once
#include "Common/Types.h"
#include "IO/FileStream.h"
#include <memory>

namespace Starshine::IO
{
	namespace File
	{
		bool Exists(std::string_view filePath);
		size_t GetSize(std::string_view filePath);

		FileStream OpenRead(std::string_view filePath);
		FileStream CreateWrite(std::string_view filePath);

		size_t ReadAllBytes(std::string_view filePath, std::unique_ptr<u8[]>& destData);
		bool WriteAllBytes(std::string_view filePath, const void* data, size_t size);

		size_t ReadAllText(std::string_view filePath, std::unique_ptr<char[]>& destData);
		std::string ReadAllText(std::string_view filePath);

		bool WriteAllText(std::string_view filePath, const char* data, size_t size);
		inline bool WriteAllText(std::string_view filePath, std::string_view text)
		{
			return WriteAllText(filePath, text.data(), text.size());
		}
	}
}
