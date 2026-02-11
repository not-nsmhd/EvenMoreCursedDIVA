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
		std::unique_ptr<Shader> LoadShader(std::string_view vsPath, std::string_view fsPath)
		{
			if (!File::Exists(vsPath) || !File::Exists(fsPath)) { return nullptr; }

			std::unique_ptr<u8[]> vsData{};
			size_t vsSize = File::ReadAllBytes(vsPath, vsData);

			if (vsSize == 0) { return nullptr; }

			std::unique_ptr<u8[]> fsData{};
			size_t fsSize = File::ReadAllBytes(fsPath, fsData);

			if (fsSize == 0) { return nullptr; }

			std::unique_ptr<Shader> shader = Rendering::GetDevice()->LoadShader(vsData.get(), vsSize, fsData.get(), fsSize);
			return shader;
		}

		std::unique_ptr<Shader> LoadShaderFromXml(std::string_view filePath)
		{
#if 0
			Xml::Document document;
			if (!Xml::ParseFromFile(document, filePath)) { return nullptr; }

			Xml::Element* root = document.RootElement();
			Xml::Element* filesElement = root->FirstChildElement("Files");

			if (filesElement != nullptr)
			{
			}

			document.Clear();
#endif
			return nullptr;
		}

		std::unique_ptr<Texture> LoadImage(std::string_view filePath)
		{
			if (!File::Exists(filePath)) { return nullptr; }

			std::unique_ptr<u8[]> imagePixels{};
			ivec2 imageSize{};
			i32 imageChannels{};

			if (ImageHelper::ReadImageFile(filePath, imageSize, imageChannels, imagePixels))
			{
				return Rendering::GetDevice()->CreateTexture(imageSize.x, imageSize.y, GFX::TextureFormat::RGBA8, imagePixels.get());
			}

			return nullptr;
		}
	}
}
