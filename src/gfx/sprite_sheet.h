#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <glm/vec2.hpp>
#include "../common/types.h"
#include "../common/rect.h"
#include "lowlevel/backend.h"
#include "sprite_renderer.h"

namespace GFX
{
	struct Sprite
	{
		std::string name;
		u32 texIndex;
		Common::RectangleF sourceRect;
		vec2 origin;
	};

	class SpriteSheet
	{
	public:
		SpriteSheet();
		
		std::string Name;

		void Destroy();

		void ReadFromTextFile(LowLevel::Backend* backend, const std::filesystem::path &dirPath);

		Sprite* GetSprite(u32 index);
		Sprite* GetSprite(std::string_view name);
		u32 GetSpriteIndex(std::string_view name);

		LowLevel::Texture* GetTexture(u32 index);
			
		void SetSpriteState(SpriteRenderer* renderer, Sprite &sprite);
		void SetSpriteState(SpriteRenderer* renderer, u32 spriteIndex);
		void SetSpriteState(SpriteRenderer* renderer, std::string_view spriteName);

		void PushSprite(SpriteRenderer* renderer, Sprite& sprite);
		void PushSprite(SpriteRenderer* renderer, Sprite* sprite);
		void PushSprite(SpriteRenderer* renderer, u32 spriteIndex);
		void PushSprite(SpriteRenderer* renderer, Sprite& sprite, vec2& scale);
		void PushSprite(SpriteRenderer* renderer, Sprite* sprite, vec2 scale);

	public:
		static constexpr u32 InvalidSpriteIndex = std::numeric_limits<u32>().max();

	private:
		LowLevel::Backend* backend;

		std::vector<Sprite> sprites;
		std::vector<LowLevel::Texture*> textures;
	};
};
