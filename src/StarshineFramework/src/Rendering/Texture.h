#pragma once
#include "GFX/Types.h"
#include "GFX/GPUResource.h"

namespace Starshine::Rendering
{
	struct Texture : public GFX::GPUResource, NonCopyable
	{
	public:
		Texture() = default;
		virtual ~Texture() = default;

		virtual i32 GetWidth() const = 0;
		virtual i32 GetHeight() const = 0;
		virtual GFX::TextureFormat GetFormat() const = 0;

		virtual void SetData(const void* source, i32 x, i32 y, i32 width, i32 height) = 0;
	};
}
