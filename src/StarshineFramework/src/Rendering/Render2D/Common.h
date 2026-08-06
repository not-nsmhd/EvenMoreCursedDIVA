#pragma once
#include "Common/Types.h"

namespace Starshine::Rendering::Render2D
{
	enum class BlendMode : u8
	{
		Disabled,
		Normal,
		Add,
		Mulitply,

		Count
	};

	constexpr std::array<std::string_view, EnumCount<BlendMode>()> BlendModeNames
	{
		"Disabled",
		"Normal",
		"Add",
		"Multiply"
	};

	struct Transform2D
	{
		vec2 Origin{ 0.5f, 0.5f };
		vec2 Position{};
		vec2 Scale{ 1.0f, 1.0f };
		f32 Rotation{};
		Color Color{ DefaultColors::White };

		constexpr Transform2D() {}
		constexpr Transform2D(const vec2& origin, const vec2& pos, const vec2& scale, const f32& rot, const Starshine::Color& color) :
			Origin{ origin },
			Position{ pos },
			Scale{ scale },
			Rotation{ rot },
			Color{ color } {};

		static constexpr Transform2D Zero()
		{
			return Transform2D({}, {}, {}, 0.0f, {});
		};
	};
}
