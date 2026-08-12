#pragma once
#include "Common/Types.h"
#include "Texture.h"
#include "IO/StreamReader.h"
#include "IO/StreamWriter.h"
#include <vector>
#include <memory>

namespace Starshine::Graphics
{
	struct GlyphKerning
	{
		i32 SecondCharCode{};
		i32 Amount{};
	};

	struct GlyphKerningList
	{
		i32 CharCode{};
		std::vector<GlyphKerning> KerningList;
	};

	struct FontGlyph
	{
		u16 X{};
		u16 Y{};
		u16 Width{};
		u16 Height{};

		i8 XOffset{};
		i8 YOffset{};
		i16 XAdvance{};

		GlyphKerningList* KerningList{};
	};

	struct GlyphRangeMapping
	{
		i32 CharCode{};
		i32 FirstGlyph{};
		size_t GlyphCount{};
	};

	enum class FontType : i32
	{
		PlainRGBA,
		SingleChannel,
		FillOutlineRG,

		Count
	};

	class Font : NonCopyable
	{
	public:
		Font() = default;
		~Font() = default;

	public:
		std::unique_ptr<Texture> TextureImage{};
		std::vector<FontGlyph> Glyphs;
		std::vector<GlyphKerningList> GlyphKernings;
		std::vector<GlyphRangeMapping> GlyphCharMappings;

		i32 Size = 0;
		i32 Baseline = 0;
		i32 LineHeight = 0;

	public:
		const FontGlyph* GetGlyph(i32 code) const;
		FontType GetType() const;

	public:
		bool ReadBMFont(std::string_view basePath, const char* xmlData, size_t xmlSize, TextureFormat texFormat);
		bool ReadBMFont(const std::string_view filePath, TextureFormat texFormat = TextureFormat::RGBA8);

	public:
		bool ReadBinary(IO::StreamReader& writer);
		void WriteBinary(IO::StreamWriter& writer);

		void Destroy();

	private:
		FontGlyph* getGlyph(i32 code);
		void resolveKerningReferences();

		FontType type{};
	};
}
