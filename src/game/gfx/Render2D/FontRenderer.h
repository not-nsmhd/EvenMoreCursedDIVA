#pragma once
#include "common/types.h"
#include "common/color.h"
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
		void PushString(const Font& font, std::string_view text, vec2& position, vec2& scale, const Common::Color& color);

	public:
		vec2 MeasureString(const Font& font, std::string_view text);

	private:
		void PushGlyph(const Font& font, const FontGlyph* glyph, vec2& position, vec2& scale, const Common::Color& color);

	private:
		SpriteRenderer& sprRenderer;
	};
}
