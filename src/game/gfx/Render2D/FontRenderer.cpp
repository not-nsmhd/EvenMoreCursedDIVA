#include "FontRenderer.h"
#include "SpriteRenderer.h"

namespace Starshine::GFX::Render2D
{
	FontRenderer::FontRenderer(SpriteRenderer& renderer) : sprRenderer(renderer)
	{
	}

	void FontRenderer::PushString(const Font& font, std::string_view text, vec2& position, vec2& scale, const Color& color)
	{
		vec2 basePos{};
		vec2 glyphOffset{};

		for (size_t i = 0; i < text.length(); i++)
		{
			char c = text.at(i);

			if (c == '\n')
			{
				basePos.x = 0.0f;
				basePos.y += font.LineHeight;
				continue;
			}

			const FontGlyph* glyph = font.GetGlyph(c);

			if (c == ' ')
			{
				basePos.x += glyph->XAdvance;
				continue;
			}

			glyphOffset.x = glyph->XOffset;
			glyphOffset.y = glyph->YOffset;
			PushGlyph(font, glyph, position + basePos + glyphOffset, scale, color);

			basePos.x += glyph->XAdvance;
		}
	}

	vec2 FontRenderer::MeasureString(const Font& font, std::string_view text)
	{
		vec2 basePos{};

		for (size_t i = 0; i < text.length(); i++)
		{
			char c = text.at(i);

			if (c == '\n')
			{
				basePos.x = 0.0f;
				basePos.y += font.LineHeight;
				continue;
			}

			const FontGlyph* glyph = font.GetGlyph(c);

			if (c == ' ')
			{
				basePos.x += glyph->XAdvance;
				continue;
			}

			basePos.x += glyph->XAdvance;
		}

		return basePos;
	}

	void FontRenderer::PushGlyph(const Font& font, const FontGlyph* glyph, vec2& position, vec2& scale, const Color& color)
	{
		float srcX = static_cast<float>(glyph->X);
		float srcY = static_cast<float>(glyph->Y);
		float srcWidth = static_cast<float>(glyph->Width);
		float srcHeight = static_cast<float>(glyph->Height);

		sprRenderer.SetSpritePosition(position);
		sprRenderer.SetSpriteScale(vec2{ srcWidth, srcHeight } *scale);
		sprRenderer.SetSpriteColor(color);

		sprRenderer.SetSpriteSource(font.Texture, RectangleF{ srcX, srcY, srcWidth, srcHeight });

		sprRenderer.SetSpriteRotation(0.0f);
		sprRenderer.SetSpriteOrigin(vec2{ 0.0f, 0.0f });

		sprRenderer.PushSprite(font.Texture);
	}
}
