#pragma once
#include "GFX/Types.h"
#include "GFX/GPUResource.h"

namespace Starshine::Rendering
{
	struct Framebuffer : public GFX::GPUResource, NonCopyable
	{
	public:
		Framebuffer() = default;
		virtual ~Framebuffer() = default;

		virtual i32 GetWidth() const = 0;
		virtual i32 GetHeight() const = 0;
		virtual GFX::TextureFormat GetFormat() const = 0;
	};
}
