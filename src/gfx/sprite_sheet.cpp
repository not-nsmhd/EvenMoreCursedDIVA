#include <fstream>
#include <SDL2/SDL.h>
#include "sprite_sheet.h"
#include "helpers/tex_helpers.h"
#include "../io/filesystem.h"
#include "../util/string_utils.h"

namespace GFX
{
	using std::string;
	using std::vector;
	using std::string_view;
	using std::unordered_map;
	using std::filesystem::path;
	using LowLevel::Backend;
	using LowLevel::Texture;

	SpriteSheet::SpriteSheet() : Name{}, backend{}, sprites{}, textures{}
	{
	}
	
	void SpriteSheet::Destroy()
	{
		for (vector<Texture*>::iterator texture = textures.begin(); texture != textures.end(); texture++)
		{
			backend->DestroyTexture(*texture);
		}

		textures.clear();
		sprites.clear();	
	}

	void SpriteSheet::ReadFromTextFile(Backend *backend, const path &dirPath)
	{
		using namespace std;
		using namespace GFX::Helpers;
		using namespace IO;

		FileSystem* fs = FileSystem::GetInstance();

		path temp = dirPath;
		temp.append("map.txt");

		path mapFilePath = fs->GetContentFilePath(temp);

		ifstream textMapFile;
		textMapFile.open(mapFilePath, ios::in | ios::binary);

		if (textMapFile.bad())
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[SpriteSheet]: Failed to load a text sprite sheet");
			return;
		}

		vector<string> keyAndValues;
		vector<string> values;

		Sprite sprite = {};
		u32 spriteIndex = 0;
		pair<string, u32> spriteMapEntry;

		u32 texCount = 1;

		for (string line; getline(textMapFile, line);)
		{
			if (line.length() > 1)
			{
				keyAndValues = Utils::String::Split(line, " = ");

				if (keyAndValues.size() == 2)
				{
					values = Utils::String::Split(keyAndValues[1], " ");

					if (values.size() == 7)
					{
						sprite.name = keyAndValues[0];
						sprite.texIndex = stoi(values[0]);
						texCount = SDL_max(texCount, sprite.texIndex + 1);

						sprite.sourceRect.x = stof(values[1]);
						sprite.sourceRect.y = stof(values[2]);
						sprite.sourceRect.width = stof(values[3]);
						sprite.sourceRect.height = stof(values[4]);

						sprite.origin.x = stof(values[5]);
						sprite.origin.y = stof(values[6]);

						sprites.push_back(sprite);

						spriteIndex++;
					}

					values.clear();
				}

				keyAndValues.clear();
			}
		}

		Name = dirPath.filename().string();
		textMapFile.close();

		if (backend != nullptr)
		{
			this->backend = backend;

			for (int i = 0; i < texCount; i++)
			{
				temp = dirPath;
				temp.append("sheet_" + to_string(i) + ".png");
				path texPath = fs->GetContentFilePath(temp);

				Texture* tex = LoadTexture(this->backend, texPath);
				textures.push_back(tex);
			}
		}
	}

	Sprite* SpriteSheet::GetSprite(u32 index)
	{
		if (index >= sprites.size())
		{
			return &sprites[0];
		}
		return &sprites[index];
	}

	Sprite* SpriteSheet::GetSprite(string_view name)
	{
		u32 index = GetSpriteIndex(name);
		return &sprites[index];
	}

	u32 SpriteSheet::GetSpriteIndex(string_view name)
	{
		for (size_t i = 0; i < sprites.size(); i++)
		{
			if (sprites[i].name == name)
			{
				return static_cast<u32>(i);
			}
		}

		return InvalidSpriteIndex;
	}

	LowLevel::Texture* SpriteSheet::GetTexture(u32 index)
	{
		if (index >= textures.size())
		{
			return textures[0];
		}
		return textures[index];
	}

	void SpriteSheet::SetSpriteState(SpriteRenderer* renderer, Sprite &sprite)
	{
		Texture* tex = textures[sprite.texIndex];
		renderer->SetSpriteOrigin(sprite.origin);
		renderer->SetSpriteScale(vec2 {sprite.sourceRect.width, sprite.sourceRect.height});
		renderer->SetSpriteSource(tex, sprite.sourceRect);
	}

	void SpriteSheet::SetSpriteState(SpriteRenderer* renderer, u32 spriteIndex)
	{
		Sprite* sprite = GetSprite(spriteIndex);
		SetSpriteState(renderer, *sprite);
	}

	void SpriteSheet::SetSpriteState(SpriteRenderer* renderer, string_view spriteName)
	{
		Sprite* sprite = GetSprite(spriteName);
		SetSpriteState(renderer, *sprite);
	}

	void SpriteSheet::PushSprite(SpriteRenderer* renderer, Sprite& sprite)
	{
		Texture* tex = textures[sprite.texIndex];
		renderer->SetSpriteOrigin(sprite.origin);
		renderer->SetSpriteScale(vec2 {sprite.sourceRect.width, sprite.sourceRect.height});
		renderer->SetSpriteSource(tex, sprite.sourceRect);
		renderer->PushSprite(tex);
	}

	void SpriteSheet::PushSprite(SpriteRenderer* renderer, Sprite* sprite)
	{
		Texture* tex = textures[sprite->texIndex];
		renderer->SetSpriteOrigin(sprite->origin);
		renderer->SetSpriteScale(vec2 {sprite->sourceRect.width, sprite->sourceRect.height});
		renderer->SetSpriteSource(tex, sprite->sourceRect);
		renderer->PushSprite(tex);
	}

	void SpriteSheet::PushSprite(SpriteRenderer* renderer, u32 spriteIndex)
	{
		Sprite* sprite = GetSprite(spriteIndex);
		PushSprite(renderer, *sprite);
	}

	void SpriteSheet::PushSprite(SpriteRenderer* renderer, Sprite &sprite, vec2& scale)
	{
		Texture* tex = textures[sprite.texIndex];
		renderer->SetSpriteOrigin(sprite.origin * scale);
		renderer->SetSpriteScale(vec2 {sprite.sourceRect.width * scale.x, sprite.sourceRect.height * scale.y});
		renderer->SetSpriteSource(tex, sprite.sourceRect);
		renderer->PushSprite(tex);
	}

	void SpriteSheet::PushSprite(SpriteRenderer* renderer, Sprite* sprite, vec2 scale)
	{
		Texture* tex = textures[sprite->texIndex];
		renderer->SetSpriteOrigin(sprite->origin * scale);
		renderer->SetSpriteScale(vec2 {sprite->sourceRect.width * scale.x, sprite->sourceRect.height * scale.y});
		renderer->SetSpriteSource(tex, sprite->sourceRect);
		renderer->PushSprite(tex);
	}
};
