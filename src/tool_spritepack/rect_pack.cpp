#include "rect_pack.h"

RectanglePacker::RectanglePacker(i32 width, i32 height, i32 paddingX, i32 paddingY)
{
	anchors.push_back({ 0, 0 });
	maxPackWidth = width;
	maxPackHeight = height;
	this->paddingX = paddingX;
	this->paddingY = paddingY;
}

RectanglePacker::~RectanglePacker()
{
	anchors.clear();
	packedSprites.clear();
}

bool RectanglePacker::TryPack(i32 width, i32 height, i32* outX, i32* outY)
{
	int paddedWidth = width + paddingX;
	int paddedHeight = height + paddingY;
	int anchorIndex = SelectAnchor(paddedWidth, paddedHeight, actualPackWidth, actualPackHeight);

	if (anchorIndex == -1)
	{
		*outX = 0;
		*outY = 0;
		return false;
	}

	Point placement = anchors[anchorIndex];
	OptimizePlacement(placement, paddedWidth, paddedHeight);

	bool blocksAnchor = ((placement.X + paddedWidth) > anchors[anchorIndex].X) && ((placement.Y + paddedHeight) > anchors[anchorIndex].Y);
	if (blocksAnchor)
	{
		anchors.erase(anchors.cbegin() + anchorIndex);
	}

	anchors.push_back({ placement.X + paddedWidth, placement.Y });
	anchors.push_back({ placement.X, placement.Y + paddedHeight });

	Sprite sprite = {};
	sprite.X = placement.X;
	sprite.Y = placement.Y;
	sprite.Width = paddedWidth;
	sprite.Height = paddedHeight;
	packedSprites.push_back(sprite);

	*outX = placement.X;
	*outY = placement.Y;
	return true;
}

void RectanglePacker::GetActualPackArea(i32* width, i32* height)
{
	*width = actualPackWidth;
	*height = actualPackHeight;
}

void RectanglePacker::OptimizePlacement(Point& placement, i32 width, i32 height)
{
	Sprite refSprite = {};
	refSprite.X = placement.X;
	refSprite.Y = placement.Y;
	refSprite.Width = width;
	refSprite.Height = height;

	int leftMost = placement.X;
	while (IsFree(refSprite, maxPackWidth, maxPackHeight))
	{
		leftMost = refSprite.X;
		refSprite.X--;
	}

	refSprite.X = placement.X;

	int topMost = placement.Y;
	while (IsFree(refSprite, maxPackWidth, maxPackHeight))
	{
		topMost = refSprite.Y;
		refSprite.Y--;
	}

	refSprite.Y = placement.Y;

	if ((placement.X - leftMost) > (placement.Y - topMost))
	{
		placement.X = leftMost;
	}
	else
	{
		placement.Y = topMost;
	}
}

i32 RectanglePacker::SelectAnchor(i32 width, i32 height, i32 testedAreaWidth, i32 testedAreaHeight)
{
	i32 freeAnchorIndex = FindFirstFreeAnchor(width, height, testedAreaWidth, testedAreaHeight);

	if (freeAnchorIndex != -1)
	{
		actualPackWidth = testedAreaWidth;
		actualPackHeight = testedAreaHeight;
		return freeAnchorIndex;
	}

	bool canEnlargeWidth = (testedAreaWidth < maxPackWidth);
	bool canEnlargeHeight = (testedAreaHeight < maxPackHeight);
	bool shouldEnlargeHeight = (!canEnlargeWidth) || (testedAreaWidth < testedAreaHeight);

	if (canEnlargeHeight && shouldEnlargeHeight)
	{
		i32 newHeight = testedAreaHeight + 4;
		return SelectAnchor(width, height, testedAreaWidth, newHeight < maxPackHeight ? newHeight : maxPackHeight);
	}
	if (canEnlargeWidth)
	{
		i32 newWidth = testedAreaWidth + 4;
		return SelectAnchor(width, height, newWidth < maxPackWidth ? newWidth : maxPackWidth, testedAreaWidth);
	}

	return -1;
}

i32 RectanglePacker::FindFirstFreeAnchor(i32 width, i32 height, i32 testedAreaWidth, i32 testedAreaHeight)
{
	Sprite potentialLocation = {};
	potentialLocation.Width = width;
	potentialLocation.Height = height;

	for (i32 i = 0; i < anchors.size(); i++)
	{
		potentialLocation.X = anchors[i].X;
		potentialLocation.Y = anchors[i].Y;

		if (IsFree(potentialLocation, testedAreaWidth, testedAreaHeight))
		{
			return i;
		}
	}

	return -1;
}

bool RectanglePacker::IsFree(Sprite& sprite, i32 testedAreaWidth, i32 testedAreaHeight)
{
	bool outsidePackingArea = (sprite.X < 0) 
		|| (sprite.Y < 0) 
		|| (sprite.X + sprite.Width > testedAreaWidth) 
		|| (sprite.Y + sprite.Height > testedAreaHeight);

	if (outsidePackingArea)
	{
		return false;
	}

	for (std::vector<Sprite>::const_iterator it = packedSprites.cbegin(); it != packedSprites.end(); it++)
	{
		if (Intersects(*it, sprite))
		{
			return false;
		}
	}

	return true;
}

bool RectanglePacker::Intersects(const Sprite& spriteA, const Sprite& spriteB)
{
	i32 spriteA_right = spriteA.X + spriteA.Width;
	i32 spriteA_bottom = spriteA.Y + spriteA.Height;

	i32 spriteB_right = spriteB.X + spriteB.Width;
	i32 spriteB_bottom = spriteB.Y + spriteB.Height;

	if ((spriteB.X < spriteA_right) && (spriteA.X < spriteB_right) &&
		(spriteB.Y < spriteA_bottom) && (spriteA.Y < spriteB_bottom))
	{
		return true;
	}

	return false;
}

void RectanglePacker::InsertAnchor(Point anchor)
{
}
