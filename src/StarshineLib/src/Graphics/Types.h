#pragma once
#include "Common/Types.h"
#include "Common/Color.h"

namespace Starshine::Graphics
{
	enum class TextureFormat : i32
	{
		Unknown = -1,

		RGBA8,
		RG8,
		R8,

		Count
	};

	constexpr std::array<size_t, EnumCount<Graphics::TextureFormat>()> TexturePixelSizes
	{
		4,
		2,
		1
	};

	constexpr size_t GetTextureDataSize(ivec2 texSize, TextureFormat format)
	{
#ifdef _MSC_VER
#pragma warning(disable : 26451)
#endif
		switch (format)
		{
		case TextureFormat::RGBA8:
			return static_cast<size_t>(texSize.x * texSize.y * 4);
		case TextureFormat::RG8:
			return static_cast<size_t>(texSize.x * texSize.y * 2);
		case TextureFormat::R8:
			return static_cast<size_t>(texSize.x * texSize.y);
		}
		return 0;
#ifdef _MSC_VER
#pragma warning(default : 26451)
#endif
	}

	struct TextureFlags
	{
		u16 WrapS : 1;
		u16 WrapT : 1;
		u16 NearestFiltering : 1;

		u16 Reserved : 13;
	};

	enum class BlendMode : i32
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
