#include "RectanglePacker.h"

// NOTE: Original algorithm developed by Javier Arevalo (https://www.flipcode.com/archives/Rectangle_Placement.shtml)

namespace Starshine::GFX
{
	static constexpr ivec2 InitialTestedAreaSize{ 128, 128 };

	void RectanglePacker::Initialize()
	{
		testedArea = Rectangle(0, 0, InitialTestedAreaSize.x, InitialTestedAreaSize.y);
		anchors.push_back({ 0, 0 });
	}

	i32 RectanglePacker::TryPack(ivec2 size)
	{
		ivec2 originalTestedAreaSize{ testedArea.Width, testedArea.Height };

		while (!AddAtEmptySpot(size))
		{
			ivec2 testAreaSize{ testedArea.Width, testedArea.Height };

			if (testAreaSize.x >= Settings.MaxSize.x || testAreaSize.y >= Settings.MaxSize.y)
			{
				testedArea.Width = originalTestedAreaSize.x;
				testedArea.Height = originalTestedAreaSize.y;
				return false;
			}

			// First try by increasing the smallest dimension
			if (testAreaSize.x < Settings.MaxSize.x && (testAreaSize.x <= testAreaSize.y || ((testAreaSize.x == testAreaSize.y) && (size.x >= size.y))))
			{
				testedArea.Width = testAreaSize.x + size.x + Settings.Padding.x;
			}
			else
			{
				testedArea.Height = testAreaSize.y + size.y + Settings.Padding.x;
			}

			if (AddAtEmptySpot(size))
			{
				return static_cast<i32>(packedRects.size()) - 1;
			}

			// Try increasing the other dimension instead
			if (testAreaSize.x != testedArea.Width)
			{
				testedArea.Width = testAreaSize.x;
				if (testAreaSize.y < Settings.MaxSize.y)
				{
					testedArea.Height = testAreaSize.y + size.y + Settings.Padding.y;
				}
			}
			else
			{
				testedArea.Height = testAreaSize.y;
				if (testAreaSize.x < Settings.MaxSize.x)
				{
					testedArea.Width = testAreaSize.x + size.x + Settings.Padding.x;
				}
			}

			if (testAreaSize.x != testedArea.Width || testAreaSize.y != testedArea.Height)
			{
				if (AddAtEmptySpot(size))
				{
					return static_cast<i32>(packedRects.size()) - 1;
				}
			}

			// Otherwise grow both, if possible, and try again
			testedArea.Width = testAreaSize.x;
			testedArea.Height = testAreaSize.y;

			if (testAreaSize.x < Settings.MaxSize.x)
			{
				testedArea.Width = testAreaSize.x + size.x + Settings.Padding.x;
			}
			if (testAreaSize.y < Settings.MaxSize.y)
			{
				testedArea.Height = testAreaSize.y + size.y + Settings.Padding.y;
			}
		}

		return static_cast<i32>(packedRects.size()) - 1;
	}

	const Rectangle& RectanglePacker::GetRectangle(i32 index) const
	{
		if (index == -1 || index >= packedRects.size())
		{
			return Rectangle();
		}
		return packedRects.at(index);
	}

	size_t RectanglePacker::GetRectangleCount() const
	{
		return packedRects.size();
	}

	ivec2 RectanglePacker::GetRealAreaSize() const
	{
		return ivec2{ testedArea.Width, testedArea.Height };
	}

	void RectanglePacker::Clear()
	{
		anchors.clear();
		packedRects.clear();
	}

	bool RectanglePacker::IsFree(i32 x, i32 y, i32 width, i32 height)
	{
		if (!testedArea.Contains(x, y, width, height))
		{
			return false;
		}

		for (auto& it : packedRects)
		{
			if (it.Intersects(x, y, width, height))
			{
				return false;
			}
		}

		return true;
	}

	bool RectanglePacker::IsFree(const Rectangle& rect) { return IsFree(rect.X, rect.Y, rect.Width, rect.Height); }

	void RectanglePacker::AddAnchor(const ivec2& position)
	{
		for (auto it = anchors.cbegin(); it != anchors.cend(); it++)
		{
			if (it->x + it->y > position.x + position.y)
			{
				anchors.insert(it, ivec2{ position.x, position.y });
				return;
			}
		}
		anchors.push_back(position);
	}

	void RectanglePacker::AddRect(const Rectangle& rect)
	{
		packedRects.push_back(rect);

		AddAnchor(ivec2{ rect.X + rect.Width + Settings.Padding.x, rect.Y });
		AddAnchor(ivec2{ rect.X, rect.Y + rect.Height + Settings.Padding.y });
	}

	bool RectanglePacker::AddAtEmptySpot(const ivec2& size)
	{
		auto erasePos = anchors.cbegin();
		bool canBePlaced = false;
		Rectangle testRect{};

		for (auto it = anchors.cbegin(); it != anchors.cend(); it++)
		{
			testRect = Rectangle(it->x, it->y, size.x, size.y);
			if (IsFree(testRect))
			{
				canBePlaced = true;
				erasePos = it;
				break;
			}
		}

		if (canBePlaced)
		{
			anchors.erase(erasePos);

			// Try to further optimize rectangle placement by shifting the rectangle
			// to the upper left corner until met with another rectangle
			int x, y {};

			for (x = 1; x <= testRect.X; x++) { if (!IsFree(x, testRect.Y, testRect.Width, testRect.Height)) { break; } }
			for (y = 1; y <= testRect.Y; y++) { if (!IsFree(testRect.X, y, testRect.Width, testRect.Height)) { break; } }

			if (y > x) { testRect.Y -= y - (Settings.Padding.y + 1); }
			else if (x > y) { testRect.X -= x - (Settings.Padding.x + 1); }

			AddRect(testRect);
		}

		return canBePlaced;
	}
}
