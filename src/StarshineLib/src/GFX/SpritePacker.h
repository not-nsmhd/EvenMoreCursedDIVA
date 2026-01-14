#pragma once
#include "Common/Types.h"
#include "Common/Rect.h"
#include "RectanglePacker.h"
#include <vector>
#include <memory>

namespace Starshine::GFX
{
	struct SpriteInfo
	{
		std::string Name;

		ivec2 Size{};
		vec2 Origin{};

		i32 DesiredTextureIndex{};

		bool WasPacked{};
		ivec2 PackedPosition{};

		i32 OriginalIndex{};

		std::string ImagePath;
	};

	struct SheetTextureInfo
	{
		ivec2 Size{};

		ivec2 RealSize{};
		size_t SpriteCount{};

		size_t DataSize{};
		std::unique_ptr<u8[]> Data;
	};

	class SpritePacker : NonCopyable
	{
	public:
		SpritePacker() = default;
		~SpritePacker() = default;

	public:
		struct SettingsData
		{
			// NOTE: Any image larger than this threshold will be rejected
			ivec2 MaxSize{ 4096, 4096 };

			// NOTE: Number of transparent pixels to add at the right and bottom sides of the sprite
			ivec2 Padding{ 1, 1 };
		} Settings;

		void Initialize();
		void Clear();

		bool AddImage(std::string_view filePath);
		void Pack();

		const SpriteInfo* GetSpriteInfo(i32 index) const;
		size_t GetSpriteCount() const;

		const SheetTextureInfo* GetTextureInfo(i32 index) const;
		size_t GetTextureCount() const;

	private:
		size_t texturesToReserve = 1;

		RectanglePacker rectPacker;

		std::vector<SpriteInfo> sprites;
		std::vector<SheetTextureInfo> textures;

		void SortSpritesByArea();
		void SortSpritesByTextureIndex();
		void RestoreOriginalSpriteOrder();
		void GenerateSheetTextures();
	};
}
