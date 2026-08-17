#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "Common/Types.h"
#include "Common/Rect.h"
#include "Texture.h"
#include "SpritePacker.h"

namespace Starshine::Graphics
{
	struct Sprite
	{
		std::string Name;
		u32 TextureIndex;
		RectangleF SourceRectangle;
		vec2 Origin;
	};

	class SpriteSheet : NonCopyable
	{
	public:
		SpriteSheet() = default;
		~SpriteSheet() = default;

	public:
		std::string Name;

		const Sprite& GetSprite(i32 index) const;
		const Sprite& GetSprite(std::string_view name) const;
		i32 GetSpriteIndex(std::string_view name) const;

		std::vector<Sprite>& GetSprites();
		const std::vector<Sprite>& GetSprites() const;
		size_t GetSpriteCount() const;

		Texture* GetTexture(i32 index) const;

	public:
		// NOTE: Sprites are intended to be packed ahead of time, hence a separate function
		// (TODO: Write an external tool for packing sprites)
		void CreateFromSpritePacker(const SpritePacker& spritePacker);
		void Clear();

	public:
		static constexpr i32 InvalidSpriteIndex = -1;

	private:
		std::vector<Sprite> sprites;
		std::vector<std::unique_ptr<Texture>> textures;
	};
};
