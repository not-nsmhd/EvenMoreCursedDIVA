#include <SDL2/SDL.h>
#include "SpriteSheet.h"
#include "IO/Path/File.h"
#include "Common/Logging/Logging.h"

namespace Starshine::Rendering::Render2D
{
	using namespace GFX;
	using std::vector;
	using std::string_view;

	void SpriteSheet::Destroy()
	{
		for (auto texture = textures.begin(); texture != textures.end(); texture++)
		{
			*texture = nullptr;
		}

		textures.clear();
		sprites.clear();	
	}

	void SpriteSheet::CreateFromSpritePacker(const GFX::SpritePacker& spritePacker)
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

		Device* device = Rendering::GetDevice();
		textures.reserve(spritePacker.GetTextureCount());
		for (size_t i = 0; i < spritePacker.GetTextureCount(); i++)
		{
			const SheetTextureInfo* texInfo = spritePacker.GetTextureInfo(static_cast<i32>(i));
			if (texInfo != nullptr)
			{
				std::unique_ptr<Texture> gpuTex = device->CreateTexture(texInfo->Size.x, texInfo->Size.y, TextureFormat::RGBA8, false, false);
				gpuTex->SetData(texInfo->Data.get(), 0, 0, texInfo->Size.x, texInfo->Size.y);
				textures.push_back(std::move(gpuTex));
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
			{
				return static_cast<i32>(i);
			}
		}

		return 0;
	}

	Texture* SpriteSheet::GetTexture(i32 index) const
	{
		if (index >= textures.size()) { return textures[0].get(); }
		return textures[index].get();
	}
};
