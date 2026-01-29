#include "OpenGLBuffers.h"

namespace Starshine::Rendering::OpenGL
{
	VertexBuffer_OpenGL::VertexBuffer_OpenGL(OpenGLDevice& device, size_t size, bool dynamic)
		: DeviceRef(device), Size(size), Dynamic(dynamic) {}

	VertexBuffer_OpenGL::~VertexBuffer_OpenGL()
	{
		glDeleteBuffersARB(1, &Handle);
	}

	void VertexBuffer_OpenGL::SetData(const void* source, size_t offset, size_t size)
	{
		if (Handle != 0 && Dynamic && source != nullptr && size + offset < Size)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, Handle);
			glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset, size, source);
		}
	}

	IndexBuffer_OpenGL::IndexBuffer_OpenGL(OpenGLDevice& device, size_t size, IndexFormat format, bool dynamic)
		: DeviceRef(device), Size(size), Format(format), Dynamic(dynamic) {}

	IndexBuffer_OpenGL::~IndexBuffer_OpenGL()
	{
		glDeleteBuffersARB(1, &Handle);
	}

	void IndexBuffer_OpenGL::SetData(const void* source, size_t offset, size_t size)
	{
		if (Handle != 0 && Dynamic && source != nullptr && size + offset < Size)
		{
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, Handle);
			glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, offset, size, source);
		}
	}
}
