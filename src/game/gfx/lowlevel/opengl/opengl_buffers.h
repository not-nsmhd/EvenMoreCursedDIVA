#pragma once
#include <glad/glad.h>
#include "../buffers.h"

namespace GFX::LowLevel::OpenGL_ARB
{
	class Buffer_OpenGL : public Buffer
	{
	public:
		Buffer_OpenGL() {}

		u32 GetSize() const;
		BufferType GetType() const;
		BufferUsage GetUsage() const;
		IndexFormat GetIndexFormat() const;

		GLuint GetHandle() const;

		bool Initialize(BufferType type, BufferUsage usage, size_t initialSize, const void* initialData);
		bool Initialize(BufferType type, BufferUsage usage, IndexFormat indexFormat, size_t initialSize, const void* initialData);
		void Destroy();

		void Bind() const;

		void SetData(const void* src, size_t offset, size_t size);

		void* Map(BufferMapping mapping);
		bool Unmap();

	private:
		GLuint handle = 0;

		bool mapped = false;
		void* mappedAddress = nullptr;

		GLenum binding = 0;
		GLenum mode = 0;
	};
}
