#pragma once
#include "gfx/Renderer.h"
#include <vector>

namespace Starshine::GFX::Render2D
{
	struct FontGlyph
	{
		i32 CharacterCode;

		u16 X;
		u16 Y;
		u16 Width;
		u16 Height;

		i8 XOffset;
		i8 YOffset;
		u16 XAdvance;
	};

	class Font
	{
	public:
		Font() = default;
		Font(const Font& other) = delete;
		~Font() = default;

	public:
		Texture* Texture = nullptr;
		std::vector<FontGlyph> Glyphs;
		i32 LineHeight = 0;

	public:
		const FontGlyph* GetGlyph(i32 code) const;

	public:
		bool ReadBMFont(std::string_view basePath, const u8* xmlData, size_t xmlSize);
		bool ReadBMFont(const std::string_view filePath);

		void Destroy();
	};
}
