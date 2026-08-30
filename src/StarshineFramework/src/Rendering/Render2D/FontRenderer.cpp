#include "FontRenderer.h"
#include "SpriteRenderer.h"
#include "Rendering/Utilities.h"
#include <utf8.h>

using namespace Starshine::Graphics;

namespace Starshine::Rendering::Render2D
{
	FontRenderer::FontRenderer(SpriteRenderer& renderer) : sprRenderer(renderer)
	{
	}

	void FontRenderer::LoadResources(Device* device)
	{
		Utilities::LoadShader("shaders/d3d11/VS_SpriteDefault.cso", "shaders/d3d11/FS_Font.cso", fontShader);
	}

	void FontRenderer::PushString(const Font* font, std::string_view text, const vec2& position, const vec2& scale,
		const Color& fillColor, const Color& outlineColor)
	{
		vec2 basePos{};
		vec2 glyphOffset{};

		auto c = text.cbegin();
		const FontGlyph* prevGlyph{};

		if (previousFont != font || !doNotSetShaderOnNextCall)
		{
			FontUniforms.FontType = static_cast<i32>(font->GetType());
			FontUniforms.OutlineColor_Vec4 = outlineColor.ToVector4();
			sprRenderer.SetShader(fontShader.get(), &FontUniforms, sizeof(FontUniforms));

			previousFont = font;
			doNotSetShaderOnNextCall = true;
		}

		while (c != text.cend())
		{
			i32 utfChar = utf8::next(c, text.cend());

			if (utfChar == '\n')
			{
				basePos.x = 0.0f;
				basePos.y += font->LineHeight * scale.y;
				prevGlyph = nullptr;
				continue;
			}

			const FontGlyph* glyph = font->GetGlyph(utfChar);

			if (utfChar == ' ')
			{
				basePos.x += glyph->XAdvance * scale.x;
				continue;
			}

			glyphOffset.x = glyph->XOffset * scale.x;
			glyphOffset.y = glyph->YOffset * scale.y;

			if (prevGlyph != nullptr && prevGlyph->KerningList != nullptr)
			{
				for (const auto& kern : prevGlyph->KerningList->KerningList)
				{
					if (kern.SecondCharCode == utfChar)
						basePos.x += kern.Amount;
				}
			}

			PushGlyph(font, glyph, position + basePos + glyphOffset, scale, fillColor);

			basePos.x += glyph->XAdvance * scale.x;
			prevGlyph = glyph;
		}
	}

	vec2 FontRenderer::MeasureString(const Font* font, std::string_view text)
	{
		return font->MeasureString(text);
	}

	void FontRenderer::PushGlyph(const Font* font, const FontGlyph* glyph, const vec2& position, const vec2& scale, const Color& color)
	{
		float srcX = static_cast<float>(glyph->X);
		float srcY = static_cast<float>(glyph->Y);
		float srcWidth = static_cast<float>(glyph->Width);
		float srcHeight = static_cast<float>(glyph->Height);

		sprRenderer.SetSpritePosition(position);
		sprRenderer.SetSpriteSize(vec2{ srcWidth, srcHeight } * scale);
		sprRenderer.SetSpriteColor(color);

		sprRenderer.SetSpriteSource(font->TextureImage.get(), RectangleF{ srcX, srcY, srcWidth, srcHeight });

		sprRenderer.SetSpriteRotation(0.0f);
		sprRenderer.SetSpriteOrigin(vec2{ 0.0f, 0.0f });

		sprRenderer.PushSprite(font->TextureImage.get());
	}
}
