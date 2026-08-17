#include <SDL2/SDL.h>
#include "SpriteSheet.h"
#include "IO/Path/File.h"
#include "Common/Logging/Logging.h"

namespace Starshine::Graphics
{
	using std::vector;
	using std::string_view;

	void SpriteSheet::Clear()
	{
		textures.clear();
		sprites.clear();	
	}

	void SpriteSheet::CreateFromSpritePacker(const SpritePacker& spritePacker)
	{
		sprites.reserve(spritePacker.GetSpriteCount());
		for (size_t i = 0; i < spritePacker.GetSpriteCount(); i++)
		{
			const SpriteInfo* sprite = spritePacker.GetSpriteInfo(static_cast<i32>(i));
			if (!sprite->WasPacked) { continue; }

			sprites.emplace_back(Sprite
				{
					sprite->Name,
					static_cast<u32>(sprite->DesiredTextureIndex),

					RectangleF(static_cast<f32>(sprite->PackedPosition.x),
					static_cast<f32>(sprite->PackedPosition.y),
					static_cast<f32>(sprite->Size.x),
					static_cast<f32>(sprite->Size.y)),

					sprite->Origin
				});
		}

		textures.reserve(spritePacker.GetTextureCount());
		for (size_t i = 0; i < spritePacker.GetTextureCount(); i++)
		{
			const SheetTextureInfo* texInfo = spritePacker.GetTextureInfo(static_cast<i32>(i));
			if (texInfo != nullptr)
			{
				/*std::unique_ptr<Texture> tex{};
				if (device->CreateTexture(texInfo->Size.x, texInfo->Size.y, TextureFormat::RGBA8, texInfo->Data.get(), gpuTex))
				{
					textures.push_back(std::move(gpuTex));
				}*/

				textures.push_back(std::make_unique<Texture>(texInfo->Size, TextureFormat::RGBA8, TextureFlags{}, texInfo->Data.get()));
			}
		}
	}

	const Sprite& SpriteSheet::GetSprite(i32 index) const
	{
		if (index >= sprites.size() || index < 0) { return sprites[0]; }
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
				return static_cast<i32>(i);
		}

		return 0;
	}

	Texture* SpriteSheet::GetTexture(i32 index) const
	{
		if (index >= textures.size()) { return textures[0].get(); }
		return textures[index].get();
	}

	std::vector<Sprite>& SpriteSheet::GetSprites()
	{
		return sprites;
	}

	const std::vector<Sprite>& SpriteSheet::GetSprites() const
	{
		return sprites;
	}

	size_t SpriteSheet::GetSpriteCount() const
	{
		return sprites.size();
	}
};
