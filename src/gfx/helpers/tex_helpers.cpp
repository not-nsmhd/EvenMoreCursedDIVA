#include "tex_helpers.h"
#include "../../common/int_types.h"
#include "../../io/filesystem.h"
#include <fstream>
#include <stb_image.h>

using namespace std::filesystem;
using namespace IO;
using std::fstream;
using std::ios;

namespace GFX
{
	namespace Helpers
	{
		LowLevel::Texture* LoadTexture(LowLevel::Backend* backend, const std::filesystem::path& path)
		{
			if (path.empty() || backend == nullptr)
			{
				return nullptr;
			}

			FileSystem* fs = FileSystem::GetInstance();
			std::filesystem::path fullPath = fs->GetContentFilePath(path);

			if (fullPath.empty())
			{
				return nullptr;
			}

			fstream imageFile;
			imageFile.open(fullPath, ios::in | ios::binary);

			if (imageFile.bad())
			{
				return nullptr;
			}

			int imageFileSize = static_cast<int>(file_size(fullPath));
			u8* imageFileData = new u8[imageFileSize];
			imageFile.read((char*)imageFileData, imageFileSize);
			imageFile.close();

			int width, height, channels;
			u8* texData = stbi_load_from_memory(imageFileData, imageFileSize, &width, &height, &channels, 4);
			delete[] imageFileData;

			if (texData == NULL)
			{
				return nullptr;
			}

			LowLevel::Texture* tex = backend->CreateTexture(width, height, LowLevel::TextureFormat::TEXFORMAT_RGBA8, 0);
			backend->SetTextureData(tex, texData);
			
			stbi_image_free(texData);
			return tex;
		}
	}
}