#pragma once
#include "common/rect.h"
#include "common/color.h"
#include "gfx/Renderer.h"
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
		void SetSpritePosition(vec2& position);
		void SetSpriteScale(vec2& absScale);
		void SetSpriteOrigin(vec2& origin);
		void SetSpriteRotation(float radians);

		// NOTE: Source must be specified in texture space
		void SetSpriteSource(Common::RectangleF& source);
		void SetSpriteSource(const Texture* texture, Common::RectangleF& absSource);

		void SetSpriteFlip(bool flipHorizontal, bool flipVertical);
		void SetSpriteColor(Common::Color color);
		
		// NOTE: Coloring order: top-left, top-right, bottom-left, bottom-right
		void SetSpriteColors(const Common::Color colors[4]);
		void SetSpriteColors(Common::Color& topLeft, Common::Color& topRight, Common::Color& bottomLeft, Common::Color& bottomRight);

		void SetBlendMode(BlendMode mode);

		void PushSprite(Texture* texture);

		void RenderSprites(Shader* shader);

	public:
		FontRenderer& Font();

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
};
