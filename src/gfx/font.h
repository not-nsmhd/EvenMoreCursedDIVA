#pragma once
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <string_view>
#include "../common/types.h"
#include "../common/color.h"
#include "lowlevel/backend.h"
#include "sprite_renderer.h"

namespace GFX
{
	struct FontGlyph
	{
		u32 CharCode;

		u16 X;
		u16 Y;
		u16 Width;
		u16 Height;

		i16 XOffset;
		i16 YOffset;
		i16 XAdvance;
	};

	struct KerningPair
	{
		u16 First;
		u16 Second;
		i16 Amount;
	};

	class Font
	{
	public:
		Font();

		std::string Name;
		std::string Typeface;
		float LineHeight = 0.0f;

		void Destroy();

		void LoadBMFont(LowLevel::Backend *backend, const std::filesystem::path& filePath);

		void PushString(SpriteRenderer* renderer, std::string_view text, vec2 pos, vec2 scale, Common::Color color);
		void PushString(SpriteRenderer* renderer, std::u16string_view text, vec2 pos, vec2 scale, Common::Color color);
		void PushString(SpriteRenderer* renderer, const char *text, size_t maxLen, vec2 pos, vec2 scale, Common::Color color);
		void PushString(SpriteRenderer* renderer, const char16_t *text, size_t maxLen, vec2 pos, vec2 scale, Common::Color color);

		void PushUTF8String(SpriteRenderer* renderer, const u8 *text, size_t size, vec2 pos, vec2 scale, Common::Color color);

	private:
		static constexpr u32 ReplacementCharcode = std::numeric_limits<u32>().max();

		LowLevel::Backend *backend = nullptr;

		std::vector<FontGlyph> glyphs;
		std::unordered_map<u32, i32> glyphMap;
		LowLevel::Texture *texture;

		FontGlyph replacementGlyph;

		void PushChar(SpriteRenderer* renderer, u32 c, vec2 basePos, vec2* charPos, vec2 scale, Common::Color color);
	};
};
