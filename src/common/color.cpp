#include <cmath>
#include "color.h"

namespace Common
{
	struct Color HSVtoRGBA(HSVColor hsv)
	{
		int hi = static_cast<int>(floorf(hsv.Hue / 60.0f)) % 6;
		float f = hsv.Hue / 60.0f - floorf(hsv.Hue / 60.0f);

		float value = hsv.Value * 255.0f;

		int v = static_cast<int>(value);
		int p = static_cast<int>(value * (1.0f - hsv.Saturation));
		int q = static_cast<int>(value * (1.0f - f * hsv.Saturation));
		int t = static_cast<int>(value * (1.0f - (1.0f - f) * hsv.Saturation));

		u8 r = 0;
		u8 g = 0;
		u8 b = 0;
		u8 a = (u8)(hsv.Alpha * 255);

		switch (hi)
		{
		case 0:
			r = (u8)(v);
			g = (u8)(t);
			b = (u8)(p);
			break;
		case 1:
			r = (u8)(q);
			g = (u8)(v);
			b = (u8)(p);
			break;
		case 2:
			r = (u8)(p);
			g = (u8)(v);
			b = (u8)(t);
			break;
		case 3:
			r = (u8)(p);
			g = (u8)(q);
			b = (u8)(v);
			break;
		case 4:
			r = (u8)(t);
			g = (u8)(p);
			b = (u8)(v);
			break;
		case 5:
			r = (u8)(v);
			g = (u8)(p);
			b = (u8)(q);
			break;
		}

		return Color(r, g, b, a);
	};
};
