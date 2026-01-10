#pragma once
#include <Common/Types.h>
#include <Common/Color.h>
#include "Font.h"

namespace Starshine::GFX::Render2D
{
	class SpriteRenderer;

	class FontRenderer
	{
	public:
		FontRenderer(SpriteRenderer& renderer);
		~FontRenderer() = default;

	public:
		void PushString(const Font& font, std::string_view text, vec2& position, vec2& scale, const Color& color);

	public:
		vec2 MeasureString(const Font& font, std::string_view text);

	private:
		void PushGlyph(const Font& font, const FontGlyph* glyph, vec2& position, vec2& scale, const Color& color);

	private:
		SpriteRenderer& sprRenderer;
	};
}
