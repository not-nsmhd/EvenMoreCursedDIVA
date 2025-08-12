#pragma once
#include "int_types.h"

struct Sprite
{
	u32 Index;

	u32 TexIndex;
	i32 X;
	i32 Y;
	i32 Width;
	i32 Height;

	float OriginX;
	float OriginY;

	bool WasPacked;
};

bool CompareByHeight(const Sprite& a, const Sprite& b);
bool CompareByArea(const Sprite& a, const Sprite& b);
bool CompareByArea_Inverted(const Sprite& a, const Sprite& b);
bool CompareByIndex(const Sprite& a, const Sprite& b);

struct Point
{
	i32 X;
	i32 Y;
};