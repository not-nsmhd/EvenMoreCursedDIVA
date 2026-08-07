#include "ImageHelper.h"
#include "IO/Path/File.h"

#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_HDR
#define STBI_NO_GIF
#define STBI_NO_PNM
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace Starshine::IO;
using namespace Starshine::Graphics;

namespace Starshine::Misc
{
	namespace ImageHelper
	{
		namespace Detail
		{
			int STBIRead_IStream(void* streamPtr, char* output, int readSize)
			{
				IStream* stream = static_cast<IStream*>(streamPtr);
				size_t readBytes = stream->ReadBuffer(output, static_cast<size_t>(readSize));
				return static_cast<int>(readBytes);
			}

			void STBISkip_IStream(void* streamPtr, int bytes)
			{
				IStream* stream = static_cast<IStream*>(streamPtr);

				size_t pos = stream->GetPosition();
				if (bytes > 0)
					stream->Seek(pos + static_cast<size_t>(bytes));
				else if (bytes < 0)
					stream->Seek(pos - static_cast<size_t>(-bytes));
			}

			int STBIEndOfFile_IStream(void* streamPtr)
			{
				IStream* stream = static_cast<IStream*>(streamPtr);
			
				if (stream->GetPosition() == stream->GetSize())
					return 1;

				return 0;
			}

			constexpr stbi_io_callbacks STBICallbacks_IStream{ STBIRead_IStream, STBISkip_IStream, STBIEndOfFile_IStream };
		}

		bool GetImageInfo(std::string_view filePath, ivec2& size, i32* channels)
		{
			int x, y, comp{};

			if (stbi_info(filePath.data(), &x, &y, &comp) == 0)
				return false;

			size.x = x;
			size.y = y;
			if (channels != nullptr) { *channels = comp; };

			return true;
		}

		bool ReadImageFile(const void* fileData, size_t fileSize, ivec2& size, i32& channels, std::unique_ptr<u8[]>& outRGBAdata)
		{
			if (fileData == nullptr || fileSize == 0)
				return false;

			constexpr int rgbaPixelSize = 4;

			int x, y, comp{};
			u8* decodedPixels = stbi_load_from_memory(reinterpret_cast<const u8*>(fileData), static_cast<int>(fileSize), &x, &y, &comp, rgbaPixelSize);

			if (decodedPixels == nullptr)
				return false;

			size.x = x;
			size.y = y;
			channels = comp;

			size_t dataSize = (x * y * 4);
			if (outRGBAdata == nullptr)
				outRGBAdata = std::make_unique<u8[]>(dataSize);

			SDL_memcpy(outRGBAdata.get(), decodedPixels, dataSize);

			stbi_image_free(decodedPixels);
			return true;
		}

		bool ReadImageFile(std::string_view filePath, ivec2& size, i32& channels, std::unique_ptr<u8[]>& outRGBAdata)
		{
			constexpr int rgbaPixelSize = 4;

			FileStream fileStream = File::OpenRead(filePath);
			if (!fileStream.IsOpen())
				return false;

			int x, y, comp{};
			u8* decodedPixels = stbi_load_from_callbacks(&Detail::STBICallbacks_IStream, &fileStream, &x, &y, &comp, rgbaPixelSize);

			if (decodedPixels == nullptr)
				return false;

			size.x = x;
			size.y = y;
			channels = comp;

			size_t dataSize = (x * y * 4);
			if (outRGBAdata == nullptr)
				outRGBAdata = std::make_unique<u8[]>(dataSize);

			SDL_memcpy(outRGBAdata.get(), decodedPixels, dataSize);

			stbi_image_free(decodedPixels);
			return true;
		}

		bool ReadImageFile(const void* fileData, size_t fileSize, std::unique_ptr<Texture>& outTexture)
		{
			ivec2 texSize{};
			i32 channels{};
			std::unique_ptr<u8[]> texData{};

			if (!ReadImageFile(fileData, fileSize, texSize, channels, texData))
				return false;

			if (outTexture == nullptr)
				outTexture = std::make_unique<Texture>(texSize, TextureFormat::RGBA8, TextureFlags{}, texData.get(), false);
			else
				outTexture->SetData({}, texSize, texData.get());

			return true;
		}

		bool ReadImageFile(std::string_view filePath, std::unique_ptr<Texture>& outTexture)
		{
			ivec2 texSize{};
			i32 channels{};
			std::unique_ptr<u8[]> texData{};

			if (!ReadImageFile(filePath, texSize, channels, texData))
				return false;

			if (outTexture == nullptr)
				outTexture = std::make_unique<Texture>(texSize, TextureFormat::RGBA8, TextureFlags{}, texData.get(), false);
			else
				outTexture->SetData({}, texSize, texData.get());

			return true;
		}
	}
}
