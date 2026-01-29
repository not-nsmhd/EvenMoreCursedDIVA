#pragma once
#include "Common/Types.h"
#include "GFX/GPUResource.h"

namespace Starshine::Rendering
{
	struct Shader : public GFX::GPUResource, NonCopyable
	{
	public:
		Shader() = default;

		// NOTE: Count is the amount of 4-dimensional F32 vectors
		virtual void SetVertexShaderVariables(u32 index, size_t count, const f32* values) = 0;
		// NOTE: Count is the amount of 4-dimensional F32 vectors
		virtual void SetFragmentShaderVariables(u32 index, size_t count, const f32* values) = 0;

		// NOTE: Index is the index of a single shader variable (vec4)
		virtual void SetVertexShaderMatrix(u32 index, const mat4& matrix) = 0;
		// NOTE: Index is the index of a single shader variable (vec4)
		virtual void SetFragmentShaderMatrix(u32 index, const mat4& matrix) = 0;
	};
}
