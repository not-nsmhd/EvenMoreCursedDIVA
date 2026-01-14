#include "ImageHelper.h"

#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_HDR
#define STBI_NO_GIF
#define STBI_NO_PNM
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Starshine::Misc
{
	namespace ImageHelper
	{
		bool GetImageInfo(std::string_view filePath, ivec2& size, i32* channels)
		{
			int x, y, comp{};

			if (stbi_info(filePath.data(), &x, &y, &comp) == 0)
			{
				return false;
			}

			size.x = x;
			size.y = y;
			if (channels != nullptr) { *channels = comp; };

			return true;
		}

		bool ReadImageFile(std::string_view filePath, ivec2& size, i32& channels, std::unique_ptr<u8[]>& outRGBAdata)
		{
			int x, y, comp{};
			u8* decodedPixels = stbi_load(filePath.data(), &x, &y, &comp, 4);

			if (decodedPixels == nullptr)
			{
				return false;
			}

			size.x = x;
			size.y = y;
			channels = comp;

			size_t dataSize = (x * y * 4);
			outRGBAdata = std::make_unique<u8[]>(dataSize);
			std::memcpy(outRGBAdata.get(), decodedPixels, dataSize);

			stbi_image_free(decodedPixels);
			return true;
		}

		bool ReadImageFile(const void* fileData, size_t fileSize, ivec2& size, i32& channels, std::unique_ptr<u8[]>& outRGBAdata)
		{
			if (fileData == nullptr || fileSize == 0)
			{
				return false;
			}

			int x, y, comp{};
			u8* decodedPixels = stbi_load_from_memory(reinterpret_cast<const u8*>(fileData), static_cast<int>(fileSize), &x, &y, &comp, 4);

			if (decodedPixels == nullptr)
			{
				return false;
			}

			size.x = x;
			size.y = y;
			channels = comp;

			size_t dataSize = (x * y * 4);
			outRGBAdata = std::make_unique<u8[]>(dataSize);
			std::memcpy(outRGBAdata.get(), decodedPixels, dataSize);

			stbi_image_free(decodedPixels);
			return true;
		}
	}
}
