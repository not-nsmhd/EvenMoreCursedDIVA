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
		Texture(ResourceHandle handle) : Resource(ResourceType::Texture, handle) {}

		virtual u32 GetWidth() const = 0;
		virtual u32 GetHeight() const = 0;

		virtual void SetData(u32 x, u32 y, u32 width, u32 height, const void* data) = 0;
	};
}
