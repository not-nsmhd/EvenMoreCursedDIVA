#pragma once
#include "Common/Types.h"
#include "Common/Color.h"
#include "Graphics/Font.h"
#include "Rendering/Device.h"
#include "Rendering/Buffers.h"
#include "Rendering/Shader.h"

namespace Starshine::Rendering::Render2D
{
	class SpriteRenderer;

	class FontRenderer
	{
		friend class SpriteRenderer;

	public:
		FontRenderer(SpriteRenderer& renderer);
		~FontRenderer() = default;

	public:
		// NOTE: This function flushes all pushed sprites currently stored in sprite renderer before drawing text
		void DrawString(const Graphics::Font* font, std::string_view text, const vec2& position, const vec2& scale,
			const Color& fillColor, const Color& outlineColor = DefaultColors::Transparent);

	public:
		vec2 MeasureString(const Graphics::Font* font, std::string_view text);

	private:
		void PushGlyph(const Graphics::Font* font, const Graphics::FontGlyph* glyph, const vec2& position, const vec2& scale, const Color& color);

	private:
		void LoadResources(Device* device);

		struct
		{
			i32 FontType{};
			i32 Padding[3]{};
			vec4 OutlineColor_Vec4{};
		} FontUniforms;

		SpriteRenderer& sprRenderer;

		std::unique_ptr<Shader> fontShader{};
		std::unique_ptr<Buffer> fontUniformBuffer{};
	};
}
