#include "types.h"

bool CompareByHeight(const Sprite& a, const Sprite& b)
{
	return a.Height < b.Height;
}

bool CompareByArea(const Sprite& a, const Sprite& b)
{
	return (a.Width * a.Height) < (b.Width * b.Height);
}

bool CompareByArea_Inverted(const Sprite& a, const Sprite& b)
{
	return (a.Width * a.Height) > (b.Width * b.Height);
}

bool CompareByIndex(const Sprite& a, const Sprite& b)
{
	return a.Index < b.Index;
}
