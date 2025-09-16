#pragma once
#include "common/rect.h"
#include "common/color.h"
#include "gfx/Renderer.h"
#include "SpriteSheetRenderer.h"
#include "FontRenderer.h"

namespace Starshine::GFX::Render2D
{
	enum class BlendMode : u8
	{
		Normal,
		Add,
		Mulitply,
		Overlay,

		Count
	};

	class SpriteRenderer
	{
	public:
		SpriteRenderer();
		~SpriteRenderer();

		void Destroy();

		void ResetSprite();
		void SetSpritePosition(const vec2& position);
		void SetSpriteScale(const vec2& absScale);
		void SetSpriteOrigin(const vec2& origin);
		void SetSpriteRotation(float radians);

		// NOTE: Source must be specified in texture space
		void SetSpriteSource(const Common::RectangleF& source);
		void SetSpriteSource(const Texture* texture, const Common::RectangleF& absSource);

		void SetSpriteFlip(bool flipHorizontal, bool flipVertical);
		void SetSpriteColor(const Common::Color& color);
		
		// NOTE: Coloring order: top-left, top-right, bottom-left, bottom-right
		void SetSpriteColors(const Common::Color colors[4]);
		void SetSpriteColors(Common::Color& topLeft, Common::Color& topRight, Common::Color& bottomLeft, Common::Color& bottomRight);

		void SetBlendMode(BlendMode mode);

		void PushSprite(Texture* texture);

		void RenderSprites(Shader* shader);

	public:
		SpriteSheetRenderer& SpriteSheet();
		FontRenderer& Font();

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
};
