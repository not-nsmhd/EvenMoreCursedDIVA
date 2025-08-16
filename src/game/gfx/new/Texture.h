#pragma once
#include "Resource.h"

namespace Starshine::GFX
{
	enum class TextureFormat
	{
		RGBA8,
		RG8,
		A8,

		Count
	};

	constexpr std::array<size_t, EnumCount<TextureFormat>()> TextureFormatPixelSizes =
	{
		4,
		2,
		1
	};

	struct Texture : public Resource
	{
	public:
		Texture(u32 width, u32 height, TextureFormat format, bool clamp, bool nearestFilter)
			: Resource(ResourceType::Texture), 
			Width(width), Height(height), Format(format), Clamp(clamp), NearestFiltering(nearestFilter) {}

		u32 Width = 0;
		u32 Height = 0;
		TextureFormat Format{};

		bool NearestFiltering = false;
		bool Clamp = false;

		virtual void SetData(u32 x, u32 y, u32 width, u32 height, const void* data) = 0;
	};
}
