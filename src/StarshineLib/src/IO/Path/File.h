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

		// TODO: Replace this with a "ReadAllBytes" function above
		size_t ReadAllBytes(std::string_view filePath, u8** dest);
	}
}
