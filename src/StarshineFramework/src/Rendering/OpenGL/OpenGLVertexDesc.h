#pragma once
#include "Rendering/VertexDesc.h"
#include "OpenGLDevice.h"
#include <glad/glad.h>
#include <vector>

namespace Starshine::Rendering::OpenGL
{
	struct VertexAttrib_OpenGL
	{
		VertexAttribType Type{};
		u32 Index;

		GLenum Format{};
		u32 Components{};

		GLsizei VertexSize;
		GLsizei Offset;
	};

	struct VertexDesc_OpenGL : public VertexDesc
	{
		VertexDesc_OpenGL(OpenGLDevice& device, const VertexAttrib* attribs, size_t attribCount);
		~VertexDesc_OpenGL();

		OpenGLDevice& DeviceRef;

		std::vector<VertexAttrib_OpenGL> Attribs;
	};
}
