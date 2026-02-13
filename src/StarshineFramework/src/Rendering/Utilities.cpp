#include "Utilities.h"
#include "IO/Path/File.h"
#include "IO/Xml.h"
#include "Misc/ImageHelper.h"

namespace Starshine::Rendering
{
	using namespace IO;
	using namespace Misc;

	namespace Utilities
	{
		bool LoadShader(std::string_view vsPath, std::string_view fsPath, std::unique_ptr<Shader>& shader)
		{
			if (!File::Exists(vsPath) || !File::Exists(fsPath)) { return false; }

			std::unique_ptr<u8[]> vsData{};
			size_t vsSize = File::ReadAllBytes(vsPath, vsData);

			if (vsSize == 0) { return false; }

			std::unique_ptr<u8[]> fsData{};
			size_t fsSize = File::ReadAllBytes(fsPath, fsData);

			if (fsSize == 0) { return false; }

			return Rendering::GetDevice()->CreateShader(vsData.get(), vsSize, fsData.get(), fsSize, shader);
		}

		bool LoadImage(std::string_view filePath, std::unique_ptr<Texture>& texture)
		{
			if (!File::Exists(filePath)) { return false; }

			std::unique_ptr<u8[]> imagePixels{};
			ivec2 imageSize{};
			i32 imageChannels{};

			if (ImageHelper::ReadImageFile(filePath, imageSize, imageChannels, imagePixels))
			{
				return Rendering::GetDevice()->CreateTexture(imageSize.x, imageSize.y, GFX::TextureFormat::RGBA8, imagePixels.get(), texture);
			}

			return false;
		}
	}
}
