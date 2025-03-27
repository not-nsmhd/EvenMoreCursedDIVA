#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <glm/vec2.hpp>
#include "../common/int_types.h"
#include "../common/rect.h"
#include "lowlevel/backend.h"
#include "sprite_renderer.h"

using Common::RectangleF;
using GFX::SpriteRenderer;
using GFX::LowLevel::Backend;
using GFX::LowLevel::Texture;
using glm::vec2;
using std::string;
using std::unordered_map;
using std::vector;
using std::filesystem::path;

namespace GFX
{
	struct Sprite
	{
		u32 texIndex;
		RectangleF sourceRect;
		vec2 origin;
	};

	class SpriteSheet
	{
	public:
		SpriteSheet();
		
		string Name;

		void Destroy();

		void ReadFromTextFile(Backend* backend, const path &dirPath);

		Sprite* GetSprite(u32 index);
		Sprite* GetSprite(const string &name);
		u32 GetSpriteIndex(const string &name);

		void SetSpriteState(SpriteRenderer &renderer, Sprite &sprite);
		void SetSpriteState(SpriteRenderer &renderer, u32 spriteIndex);
		void SetSpriteState(SpriteRenderer &renderer, const string &spriteName);

		void PushSprite(SpriteRenderer &renderer, Sprite& sprite);
		void PushSprite(SpriteRenderer &renderer, Sprite* sprite);
		void PushSprite(SpriteRenderer &renderer, u32 spriteIndex);
		void PushSprite(SpriteRenderer &renderer, const string& spriteName);
		void PushSprite(SpriteRenderer &renderer, Sprite& sprite, vec2& scale);
		void PushSprite(SpriteRenderer &renderer, Sprite* sprite, vec2 scale);
		void PushSprite(SpriteRenderer &renderer, const string &spriteName, vec2& scale);

	private:
		Backend* backend;

		vector<Sprite> sprites;
		unordered_map<string, u32> spriteMap;
		vector<Texture*> textures;
	};
};
