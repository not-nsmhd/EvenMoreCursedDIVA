#pragma once
#include "Rendering/Buffers.h"
#include "OpenGLDevice.h"
#include <glad/glad.h>

namespace Starshine::Rendering::OpenGL
{
	struct VertexBuffer_OpenGL : public VertexBuffer
	{
		VertexBuffer_OpenGL(OpenGLDevice& device, size_t size, bool dynamic);
		~VertexBuffer_OpenGL();

		void SetData(const void* source, size_t offset, size_t size) override;

		OpenGLDevice& DeviceRef;

		GLuint Handle = 0;
		size_t Size = 0;
		bool Dynamic = false;
	};

	struct IndexBuffer_OpenGL : public IndexBuffer
	{
		IndexBuffer_OpenGL(OpenGLDevice& device, size_t size, IndexFormat format, bool dynamic);
		~IndexBuffer_OpenGL();

		void SetData(const void* source, size_t offset, size_t size) override;

		OpenGLDevice& DeviceRef;

		GLuint Handle = 0;
		size_t Size = 0;
		bool Dynamic = false;
		IndexFormat Format = {};
	};
}
