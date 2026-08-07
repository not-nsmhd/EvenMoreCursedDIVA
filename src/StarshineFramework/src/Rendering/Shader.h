#pragma once
#include "Common/Types.h"
#include "Graphics/GPUResource.h"

namespace Starshine::Rendering
{
	enum class ShaderStage : u8
	{
		Vertex,
		Fragment,

		Count
	};

	struct Shader : public Graphics::GPUResource, NonCopyable
	{
	public:
		Shader() = default;
	};
}
