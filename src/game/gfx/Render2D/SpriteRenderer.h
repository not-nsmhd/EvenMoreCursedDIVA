#pragma once
#include <Common/Rect.h>
#include <Common/Color.h>
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

	struct SpriteVertex
	{
		vec2 Position{};
		vec2 TexCoord{};
		Color Color{};
	};

	class SpriteRenderer
	{
	public:
		SpriteRenderer();
		~SpriteRenderer();

		void Destroy();

		void ResetSprite();
		void SetSpritePosition(const vec2& position);
		void SetSpriteSize(const vec2& absScale);
		void SetSpriteOrigin(const vec2& origin);
		void SetSpriteRotation(float radians);

		// NOTE: Source must be specified in texture space
		// (texture's point at its width and height is represented as a (1.0, 1.0) coordinate)
		void SetSpriteSource(const RectangleF& texSpaceSource);
		void SetSpriteSource(const Texture* texture, const RectangleF& absSource);

		void SetSpriteFlip(bool flipHorizontal, bool flipVertical);
		void SetSpriteColor(const Color& color);
		
		// NOTE: Coloring order: top-left, top-right, bottom-left, bottom-right
		void SetSpriteColors(const Color colors[4]);
		void SetSpriteColors(const Color& topLeft, const Color& topRight, const Color& bottomLeft, const Color& bottomRight);

		void SetBlendMode(BlendMode mode);

		void PushSprite(Texture* texture);

		void RenderSprites(Shader* shader);

	public:
		void PushShape(const SpriteVertex* vertices, size_t vertexCount, GFX::PrimitiveType primType, Texture* texture);

	public:
		void PushLine(const vec2& position, float angle, float length, const Color& color, float thickness = 1.0f);
		void PushOutlineRect(const vec2& position, const vec2& size, const vec2& origin, const Color& color, float thickness = 1.0f);

	public:
		SpriteSheetRenderer& SpriteSheet();
		FontRenderer& Font();

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
};
