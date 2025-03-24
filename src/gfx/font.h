#pragma once
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <string>
#include "../common/int_types.h"
#include "../common/color.h"
#include "lowlevel/backend.h"
#include "sprite_renderer.h"

using Common::Color;
using GFX::SpriteRenderer;
using GFX::LowLevel::Backend;
using GFX::LowLevel::Texture;
using std::pair;
using std::string;
using std::u16string;
using std::unordered_map;
using std::vector;

namespace GFX
{
	struct FontGlyph
	{
		u16 CharCode;

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

		string Name;
		string Typeface;
		float LineHeight = 0.0f;

		void Destroy();

		void LoadBMFont(Backend *backend, const std::filesystem::path &path);

		void PushString(SpriteRenderer &renderer, const string &text, vec2 pos, vec2 scale, Color color);
		void PushString(SpriteRenderer &renderer, const u16string &text, vec2 pos, vec2 scale, Color color);
		void PushString(SpriteRenderer &renderer, const char *text, size_t maxLen, vec2 pos, vec2 scale, Color color);
		void PushString(SpriteRenderer &renderer, const char16_t *text, size_t maxLen, vec2 pos, vec2 scale, Color color);

	private:
		Backend *backend = nullptr;

		vector<FontGlyph> glyphs;
		unordered_map<char16_t, i32> glyphMap;
		Texture *texture;

		FontGlyph replacementGlyph;

		void PushChar(SpriteRenderer &renderer, char16_t c, vec2 basePos, vec2* charPos, vec2 scale, Color color);
	};
};
