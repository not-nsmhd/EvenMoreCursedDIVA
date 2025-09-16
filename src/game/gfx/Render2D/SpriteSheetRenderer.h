#pragma once
#include "common/types.h"
#include "common/color.h"
#include "SpriteSheet.h"

namespace Starshine::GFX::Render2D
{
	class SpriteRenderer;

	class SpriteSheetRenderer
	{
	public:
		SpriteSheetRenderer(SpriteRenderer& renderer);
		~SpriteSheetRenderer() = default;

	public:
		void SetSpriteState(const SpriteSheet& sheet, const Sprite& sprite, const vec2& scale, i32* texIndex);
		void SetSpriteState(const SpriteSheet& sheet, i32 spriteIndex, const vec2& scale, i32* texIndex);
		void SetSpriteState(const SpriteSheet& sheet, std::string_view spriteName, const vec2& scale, i32* texIndex);

		void PushSprite(const SpriteSheet& sheet, const Sprite& sprite, vec2& position, vec2& scale, const Common::Color& color);
		void PushSprite(const SpriteSheet& sheet, i32 spriteIndex, vec2& position, vec2& scale, const Common::Color& color);
		void PushSprite(const SpriteSheet& sheet, std::string_view spriteName, vec2& position, vec2& scale, const Common::Color& color);

	private:
		SpriteRenderer& sprRenderer;
	};
}
