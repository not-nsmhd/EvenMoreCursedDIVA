#pragma once
#include "common/types.h"
#include <string>

namespace Starshine::IO
{
	namespace File
	{
		bool Exists(std::string_view filePath);
		size_t GetSize(std::string_view filePath);

		// NOTE: Allocates a new "u8" buffer which must be disposed of by using "delete[]" when it's no longer needed
		size_t ReadAllBytes(std::string_view filePath, u8** dest);

		std::string_view GetParentDirectory(std::string_view path);
	}
}
