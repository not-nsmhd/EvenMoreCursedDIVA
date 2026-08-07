#include "SpriteSheetRenderer.h"
#include "SpriteRenderer.h"

using namespace Starshine::Graphics;
using StarshineTex = Starshine::Graphics::Texture;

namespace Starshine::Rendering::Render2D
{
	SpriteSheetRenderer::SpriteSheetRenderer(SpriteRenderer& renderer) : sprRenderer(renderer)
	{
	}

	void SpriteSheetRenderer::SetSpriteState(const SpriteSheet& sheet, const Sprite& sprite, const vec2& scale, i32* texIndex)
	{
		StarshineTex* tex = sheet.GetTexture(sprite.TextureIndex);
		sprRenderer.SetSpriteOrigin(sprite.Origin * scale);
		sprRenderer.SetSpriteSize({ sprite.SourceRectangle.Width * scale.x, sprite.SourceRectangle.Height * scale.y });
		sprRenderer.SetSpriteSource(tex, sprite.SourceRectangle);

		if (texIndex != nullptr)
			*texIndex = sprite.TextureIndex;
	}

	void SpriteSheetRenderer::SetSpriteState(const SpriteSheet& sheet, i32 spriteIndex, const vec2& scale, i32* texIndex)
	{
		const Sprite& sprite = sheet.GetSprite(spriteIndex);
		SetSpriteState(sheet, sprite, scale, texIndex);
	}

	void SpriteSheetRenderer::SetSpriteState(const SpriteSheet& sheet, std::string_view spriteName, const vec2& scale, i32* texIndex)
	{
		const Sprite& sprite = sheet.GetSprite(spriteName);
		SetSpriteState(sheet, sprite, scale, texIndex);
	}

	void SpriteSheetRenderer::PushSprite(const SpriteSheet& sheet, const Sprite& sprite, const vec2& position, const vec2& scale, const Color& color)
	{
		SetSpriteState(sheet, sprite, scale, nullptr);
		sprRenderer.SetSpritePosition(position);
		sprRenderer.SetSpriteColor(color);
		sprRenderer.PushSprite(sheet.GetTexture(sprite.TextureIndex));
	}

	void SpriteSheetRenderer::PushSprite(const SpriteSheet& sheet, i32 spriteIndex, const vec2& position, const vec2& scale, const Color& color)
	{
		const Sprite& sprite = sheet.GetSprite(spriteIndex);
		PushSprite(sheet, sprite, position, scale, color);
	}

	void SpriteSheetRenderer::PushSprite(const SpriteSheet& sheet, std::string_view spriteName, const vec2& position, const vec2& scale, const Color& color)
	{
		const Sprite& sprite = sheet.GetSprite(spriteName);
		PushSprite(sheet, sprite, position, scale, color);
	}
}
