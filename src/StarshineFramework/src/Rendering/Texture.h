#pragma once
#include "Graphics/Types.h"
#include "Graphics/GPUResource.h"

namespace Starshine::Rendering
{
	struct Texture : public Graphics::GPUResource, NonCopyable
	{
	public:
		virtual ~Texture() = default;
	};
}
