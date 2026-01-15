#include <fstream>
#include <SDL2/SDL.h>
#include "SpriteSheet.h"
#include "IO/Path/File.h"
#include "util/string_utils.h"
#include "util/logging.h"

namespace Starshine::GFX::Render2D
{
	using std::string;
	using std::vector;
	using std::string_view;
	using std::fstream;
	using std::ios;
	using namespace Logging;

	constexpr const char* LogName = "Starshine::GFX::Render2D::SpriteSheet";
	
	void SpriteSheet::Destroy()
	{
		Renderer* renderer = Renderer::GetInstance();

		for (vector<Texture*>::iterator texture = textures.begin(); texture != textures.end(); texture++)
		{
			renderer->DeleteResource(*texture);
		}

		textures.clear();
		sprites.clear();	
	}

	bool SpriteSheet::ReadFromTextFile(std::string_view dirPath)
	{
		std::string mapFilePath = std::string(dirPath);
		mapFilePath.append("/map.txt");

		fstream mapFileStream;
		mapFileStream.open(mapFilePath, ios::in | ios::binary);

		if (mapFileStream.bad())
		{
			LogError(LogName, "Failed to load a text sprite sheet at path %s", mapFilePath.c_str());
			return false;
		}

		Sprite sprite = {};
		u32 spriteIndex = 0;

		u32 texCount = 1;
		char spriteName[128]{};

		for (string line; std::getline(mapFileStream, line);)
		{
			if (line.length() > 1)
			{
				SDL_sscanf(line.c_str(), "%s = %d %f %f %f %f %f %f",
					&spriteName,
					&sprite.TextureIndex,
					&sprite.SourceRectangle.X, &sprite.SourceRectangle.Y, &sprite.SourceRectangle.Width, &sprite.SourceRectangle.Height,
					&sprite.Origin.x, &sprite.Origin.y);

				sprite.Name = spriteName;
				sprites.push_back(sprite);
			}
		}

		mapFileStream.close();

		Renderer* renderer = Renderer::GetInstance();

		std::string texPath;
		for (int i = 0; i < texCount; i++)
		{
			texPath = dirPath;
			texPath.append("/sheet_" + std::to_string(i) + ".png");

			Texture* tex = renderer->LoadTexture(texPath, false, true);
			if (tex != nullptr)
			{
				textures.push_back(tex);
			}
		}

		return true;
	}

	void SpriteSheet::CreateFromSpritePacker(const GFX::SpritePacker& spritePacker)
	{
		sprites.reserve(spritePacker.GetSpriteCount());
		for (size_t i = 0; i < spritePacker.GetSpriteCount(); i++)
		{
			const SpriteInfo* sprite = spritePacker.GetSpriteInfo(i);
			if (!sprite->WasPacked) { continue; }

			sprites.emplace_back(Sprite
				{
					sprite->Name,
					static_cast<u32>(sprite->DesiredTextureIndex),
					RectangleF(sprite->PackedPosition.x, sprite->PackedPosition.y, sprite->Size.x, sprite->Size.y),
					sprite->Origin
				});
		}

		textures.reserve(spritePacker.GetTextureCount());
		for (size_t i = 0; i < spritePacker.GetTextureCount(); i++)
		{
			const SheetTextureInfo* texInfo = spritePacker.GetTextureInfo(i);
			if (texInfo != nullptr)
			{
				Texture* gpuTex = Renderer::GetInstance()->CreateTexture(texInfo->Size.x, texInfo->Size.y, TextureFormat::RGBA8, false, true);
				gpuTex->SetData(0, 0, texInfo->Size.x, texInfo->Size.y, texInfo->Data.get());
				textures.push_back(gpuTex);
			}
		}
	}

	const Sprite& SpriteSheet::GetSprite(i32 index) const
	{
		if (index >= sprites.size() || index < 0)
		{
			return sprites[0];
		}
		return sprites[index];
	}

	const Sprite& SpriteSheet::GetSprite(string_view name) const
	{
		i32 index = GetSpriteIndex(name);
		return sprites[index];
	}

	i32 SpriteSheet::GetSpriteIndex(string_view name) const
	{
		for (size_t i = 0; i < sprites.size(); i++)
		{
			if (sprites[i].Name == name)
			{
				return static_cast<i32>(i);
			}
		}

		return 0;
	}

	Texture* SpriteSheet::GetTexture(i32 index) const
	{
		if (index >= textures.size())
		{
			return textures[0];
		}
		return textures[index];
	}
};
