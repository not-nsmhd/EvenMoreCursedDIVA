#pragma once
#include "GFX/Types.h"
#include "GFX/GPUResource.h"

namespace Starshine::Rendering
{
	struct Texture : public GFX::GPUResource, NonCopyable
	{
	public:
		Texture() = default;

		virtual u32 GetWidth() const = 0;
		virtual u32 GetHeight() const = 0;
		virtual GFX::TextureFormat GetFormat() const = 0;

		virtual void SetData(const void* source, u32 x, u32 y, u32 width, u32 height) = 0;
	};
}
