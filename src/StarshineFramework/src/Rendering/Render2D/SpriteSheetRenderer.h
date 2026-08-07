#pragma once
#include <Common/Types.h>
#include <Common/Color.h>
#include "Graphics/SpriteSheet.h"

namespace Starshine::Rendering::Render2D
{
	class SpriteRenderer;

	class SpriteSheetRenderer
	{
	public:
		SpriteSheetRenderer(SpriteRenderer& renderer);
		~SpriteSheetRenderer() = default;

	public:
		void SetSpriteState(const Graphics::SpriteSheet& sheet, const Graphics::Sprite& sprite, const vec2& scale, i32* texIndex);
		void SetSpriteState(const Graphics::SpriteSheet& sheet, i32 spriteIndex, const vec2& scale, i32* texIndex);
		void SetSpriteState(const Graphics::SpriteSheet& sheet, std::string_view spriteName, const vec2& scale, i32* texIndex);

		void PushSprite(const Graphics::SpriteSheet& sheet, const Graphics::Sprite& sprite, const vec2& position, const vec2& scale, const Color& color);
		void PushSprite(const Graphics::SpriteSheet& sheet, i32 spriteIndex, const vec2& position, const vec2& scale, const Color& color);
		void PushSprite(const Graphics::SpriteSheet& sheet, std::string_view spriteName, const vec2& position, const vec2& scale, const Color& color);

	private:
		SpriteRenderer& sprRenderer;
	};
}
