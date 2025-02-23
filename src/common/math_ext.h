#pragma once
#include <glm/vec2.hpp>

using glm::vec2;

namespace Common
{
	namespace MathExtensions
	{
		const float MATH_EXT_PI = 3.1415927f;
		const float MATH_EXT_2PI = 6.2831854f;

		vec2 RotateVector(vec2 point, vec2 origin, float cos, float sin);
		vec2 RotateVector(vec2 point, vec2 origin, float radians);
		float ToRadians(float degrees);
		float ToDegrees(float radians);

		float ConvertRange(float originalStart, float originalEnd, float newStart, float newEnd, float value);
		vec2 GetSinePoint(float percentage, vec2 target, float deg, float freq, float ampl, float dist);
	};
};
