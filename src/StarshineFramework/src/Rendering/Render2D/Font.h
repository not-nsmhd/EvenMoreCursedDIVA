#pragma once
#include "Rendering/Device.h"
#include <vector>

namespace Starshine::Rendering::Render2D
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

	class Font : public NonCopyable
	{
	public:
		Font() = default;
		~Font();

	public:
		std::unique_ptr<Texture> Texture{};
		std::vector<FontGlyph> Glyphs;
		i32 LineHeight = 0;

	public:
		const FontGlyph* GetGlyph(i32 code) const;

	public:
		bool ReadBMFont(std::string_view basePath, const std::unique_ptr<u8[]>& xmlData, size_t xmlSize);
		bool ReadBMFont(const std::string_view filePath);

		void Destroy();
	};
}
