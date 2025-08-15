#pragma once
#include "Resource.h"

namespace Starshine::GFX
{
	struct Shader : public Resource
	{
	public:
		Shader(void* vsData, size_t vsSize, void* fsData, size_t fsSize) 
			: Resource(ResourceType::Shader), 
			VertexShaderSource(vsData), VertexShaderSize(vsSize),
			FragmentShaderSource(fsData), FragmentShaderSize(fsSize) {}

		void* VertexShaderSource = nullptr;
		size_t VertexShaderSize = 0;

		void* FragmentShaderSource = nullptr;
		size_t FragmentShaderSize = 0;
	};
}
