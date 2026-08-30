#include "Utilities.h"
#include "IO/Path/File.h"
#include "IO/FileSystem/FileSystem.h"
#include "Misc/ImageHelper.h"

namespace Starshine::Rendering
{
	using namespace IO;
	using namespace Misc;

	namespace Utilities
	{
		bool LoadShader(std::string_view vsPath, std::string_view fsPath, std::unique_ptr<Shader>& shader)
		{
			auto fs = FileSystem::GetInstance();
			if (!fs->FileExists(vsPath) || !fs->FileExists(fsPath)) { return false; }

			std::unique_ptr<u8[]> vsData{};
			size_t vsSize = fs->ReadAllBytes(vsPath, vsData);

			if (vsSize == 0) { return false; }

			std::unique_ptr<u8[]> fsData{};
			size_t fsSize = fs->ReadAllBytes(fsPath, fsData);

			if (fsSize == 0) { return false; }

			return Rendering::GetDevice()->CreateShader(vsData.get(), vsSize, fsData.get(), fsSize, shader);
		}

		void UploadTextureToGPU(Graphics::Texture* texture, bool freeCPUData)
		{
			if (texture->GPUTexture.Resource == nullptr)
			{
				Rendering::GetDevice()->UploadTexture(texture);

				if (freeCPUData)
					texture->FreeCPUData();
			}
		}
	}
}
