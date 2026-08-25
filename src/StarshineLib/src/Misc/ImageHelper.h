#pragma once
#include "Common/Types.h"
#include "Graphics/Texture.h"
#include "IO/IStream.h"
#include <memory>

namespace Starshine::Misc
{
	namespace ImageHelper
	{
		bool GetImageInfo(std::string_view filePath, ivec2& size, i32* channels);

		bool ReadImageFile(const void* fileData, size_t fileSize, ivec2& size, i32& channels, std::unique_ptr<u8[]>& outRGBAdata);
		bool ReadImageFile(std::string_view filePath, ivec2& size, i32& channels, std::unique_ptr<u8[]>& outRGBAdata);

		bool ReadImageFile(const void* fileData, size_t fileSize, std::unique_ptr<Graphics::Texture>& outTexture, Graphics::TextureFormat targetFormat);
		bool ReadImageFile(std::string_view filePath, std::unique_ptr<Graphics::Texture>& outTexture,
			Graphics::TextureFormat targetFormat = Graphics::TextureFormat::RGBA8);

		enum class ImageFormat : i32
		{
			PNG,
			TGA,

			Count
		};

		constexpr std::array<std::string_view, EnumCount<ImageFormat>()> ImageFormatNames
		{
			"PNG",
			"TGA"
		};

		void WriteImageFile(IO::IStream& outputStream, ImageFormat outputFormat, const u8* data, ivec2 size, Graphics::TextureFormat format);
		void WriteImageFile(std::string_view filePath, ImageFormat outputFormat, const u8* data, ivec2 size, Graphics::TextureFormat format);
	}
}
