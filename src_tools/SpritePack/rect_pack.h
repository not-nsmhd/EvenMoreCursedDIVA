#pragma once
#include <string>
#include <vector>
#include "types.h"

class RectanglePacker
{
public:
	RectanglePacker() = delete;
	RectanglePacker(i32 width, i32 height, i32 paddingX, i32 paddingY);
	~RectanglePacker();

	bool TryPack(i32 width, i32 height, i32* outX, i32* outY);

	void GetActualPackArea(i32* width, i32* height);
private:
	i32 actualPackWidth = 1;
	i32 actualPackHeight = 1;

	i32 paddingX = 0;
	i32 paddingY = 0;

	i32 maxPackWidth = 1;
	i32 maxPackHeight = 1;

	std::vector<Point> anchors = {};
	std::vector<Sprite> packedSprites = {};

	void OptimizePlacement(Point& placement, i32 width, i32 height);
	i32 SelectAnchor(i32 width, i32 height, i32 testedAreaWidth, i32 testedAreaHeight);
	i32 FindFirstFreeAnchor(i32 width, i32 height, i32 testedAreaWidth, i32 testedAreaHeight);
	bool IsFree(Sprite& sprite, i32 testedAreaWidth, i32 testedAreaHeight);
	bool Intersects(const Sprite& spriteA, const Sprite& spriteB);
	void InsertAnchor(Point anchor);
};
