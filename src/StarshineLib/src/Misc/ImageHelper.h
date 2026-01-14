#pragma once
#include "Common/Types.h"
#include <memory>

namespace Starshine::Misc
{
	namespace ImageHelper
	{
		bool GetImageInfo(std::string_view filePath, ivec2& size, i32* channels);

		bool ReadImageFile(std::string_view filePath, ivec2& size, i32& channels, std::unique_ptr<u8[]>& outRGBAdata);
		bool ReadImageFile(const void* fileData, size_t fileSize, ivec2& size, i32& channels, std::unique_ptr<u8[]>& outRGBAdata);
	}
}
