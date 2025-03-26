#include <SDL2/SDL_stdinc.h>
#include <glm/glm.hpp>
#include "math_ext.h"

namespace Common
{
	namespace MathExtensions
	{
		vec2 RotateVector(vec2 point, vec2 origin, float cos, float sin)
		{
			vec2 p = point - origin;

			float x = p.x * cos - p.y * sin;
			float y = p.x * sin + p.y * cos;

			return { x, y };
		}

		vec2 RotateVector(vec2 point, vec2 origin, float radians)
		{
			float cos = SDL_cosf(radians);
			float sin = SDL_sinf(radians);
			return RotateVector(point, origin, cos, sin);
		}

		float ToRadians(float degrees)
		{
			return glm::radians<float>(degrees);
		}

		float ToDegrees(float radians)
		{
			return glm::degrees<float>(radians);
		}
		
		float ConvertRange(float originalStart, float originalEnd, float newStart, float newEnd, float value)
		{
			return newStart + (value - originalStart) * (newEnd - newStart) / (originalEnd - originalStart);
		}
		
		vec2 GetSinePoint(float percentage, vec2 target, float deg, float freq, float ampl, float dist)
		{
			if (dist == 0.0f)
			{
				return target;
			}

			if (SDL_fmodf(freq, 2.0f) != 0.0f)
			{
				freq *= -1.0f;
			}

			float x = percentage * dist;
			float y = SDL_sinf(percentage * MATH_EXT_PI * freq) / 12.0f * ampl;
			float radians = glm::radians<float>(deg - 90.0f);

			vec2 point = RotateVector(vec2(x, y), vec2(0.0f), radians);
			return point + target;
		}
		
		bool IsInRange(float start, float end, float value)
		{
			return ((value >= start) && (value <= end));
		}
		
		float CalculateBarDuration_Seconds(float bpm, int beatsPerBar)
		{
			if (bpm <= 0.0f || beatsPerBar <= 0)
			{
				return 0.0f;
			}

			return (60.0f / bpm * static_cast<float>(beatsPerBar));
		}
	};
};
