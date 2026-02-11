#pragma once
#include "Common/Types.h"
#include "GFX/GPUResource.h"

namespace Starshine::Rendering
{
	enum class ShaderStage : u8
	{
		Vertex,
		Fragment,

		Count
	};

	struct Shader : public GFX::GPUResource, NonCopyable
	{
	public:
		Shader() = default;
	};
}
