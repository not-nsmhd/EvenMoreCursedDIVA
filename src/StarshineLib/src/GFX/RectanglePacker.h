#pragma once
#include "Common/Types.h"
#include "Common/Rect.h"
#include <vector>

namespace Starshine::GFX
{
	class RectanglePacker : NonCopyable
	{
	public:
		RectanglePacker() = default;
		~RectanglePacker() = default;

	public:
		struct SettingsData
		{
			ivec2 MaxSize{ 4096, 4096 };
			ivec2 Padding{ 1, 1 };
		} Settings;

		void Initialize();

		// NOTE: Attempts to place a new rectangle into the area. Returns a rectangle index on success, or -1 on fail.
		i32 TryPack(ivec2 size);

		const Rectangle& GetRectangle(i32 index) const;
		size_t GetRectangleCount() const;

		ivec2 GetRealAreaSize() const;

		void Clear();

	private:
		std::vector<ivec2> anchors;
		std::vector<Rectangle> packedRects;

		Rectangle testedArea{};

		bool IsFree(i32 x, i32 y, i32 width, i32 height);
		bool IsFree(const Rectangle& rect);
		void AddAnchor(const ivec2& position);
		void AddRect(const Rectangle& rect);
		bool AddAtEmptySpot(const ivec2& size);
	};
}
