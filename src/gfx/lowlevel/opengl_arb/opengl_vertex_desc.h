#pragma once
#include <glad/glad.h>
#include "../vertex_desc.h"

namespace GFX::LowLevel::OpenGL_ARB
{
	class VertexDescription_OpenGL : public VertexDescription
	{
	public:
		VertexDescription_OpenGL() {}

		const VertexAttribute *GetAttributes() const;
		size_t GetAttributeCount() const;
		size_t GetVertexStride() const;

		bool Initialize(const VertexAttribute *attribs, u32 attribCount, size_t stride);
		void Destroy();

		void Set() const;
	};
}
