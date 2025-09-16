#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <glm/vec2.hpp>
#include "common/types.h"
#include "common/rect.h"
#include "gfx/Renderer.h"

namespace Starshine::GFX::Render2D
{
	struct Sprite
	{
		std::string Name;
		u32 TextureIndex;
		Common::RectangleF SourceRectangle;
		vec2 Origin;
	};

	class SpriteSheet
	{
	public:
		SpriteSheet() = default;
		SpriteSheet(const SpriteSheet& other) = delete;
		~SpriteSheet() = default;

	public:
		std::string Name;

		const Sprite& GetSprite(i32 index) const;
		const Sprite& GetSprite(std::string_view name) const;
		i32 GetSpriteIndex(std::string_view name) const;

		Texture* GetTexture(i32 index) const;

	public:
		bool ReadFromTextFile(std::string_view dirPath);
		void Destroy();

	public:
		static constexpr i32 InvalidSpriteIndex = -1;

	private:
		std::vector<Sprite> sprites;
		std::vector<Texture*> textures;
	};
};
