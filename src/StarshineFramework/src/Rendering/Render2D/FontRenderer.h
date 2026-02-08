#pragma once
#include "Common/Types.h"
#include "Common/Color.h"
#include "Font.h"

namespace Starshine::Rendering::Render2D
{
	class SpriteRenderer;

	class FontRenderer
	{
	public:
		FontRenderer(SpriteRenderer& renderer);
		~FontRenderer() = default;

	public:
		void PushString(const Font* font, std::string_view text, const vec2& position, const vec2& scale, const Color& color);

	public:
		vec2 MeasureString(const Font* font, std::string_view text);

	private:
		void PushGlyph(const Font* font, const FontGlyph* glyph, const vec2& position, const vec2& scale, const Color& color);

	private:
		SpriteRenderer& sprRenderer;
	};
}
