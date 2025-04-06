#include "opengl_buffers.h"
#include "opengl_defs.h"

namespace GFX::LowLevel::OpenGL_ARB
{
	namespace GFXtoGL
	{
		GLenum BufferBinding[] =
			{
				GL_ARRAY_BUFFER_ARB,		// BUFFER_VERTEX
				GL_ELEMENT_ARRAY_BUFFER_ARB // BUFFER_INDEX
		};

		GLenum BufferMode[] =
			{
				GL_STATIC_DRAW_ARB, // BUFFER_USAGE_STATIC
				GL_DYNAMIC_DRAW_ARB // BUFFER_USAGE_DYNAMIC
		};

		GLenum BufferMapping[] =
			{
				GL_READ_ONLY_ARB,  // BUFFER_MAPPING_READ
				GL_WRITE_ONLY_ARB, // BUFFER_MAPPING_WRITE
				GL_READ_WRITE_ARB  // BUFFER_MAPPING_READWRITE
		};
	}

	u32 Buffer_OpenGL::GetSize() const
	{
		return size;
	}

	BufferType Buffer_OpenGL::GetType() const
	{
		return type;
	}

	BufferUsage Buffer_OpenGL::GetUsage() const
	{
		return usage;
	}

	IndexFormat Buffer_OpenGL::GetIndexFormat() const
	{
		return indexFormat;
	}

	GLuint Buffer_OpenGL::GetHandle() const
	{
		return handle;
	}

	bool Buffer_OpenGL::Initialize(BufferType type, BufferUsage usage, size_t initialSize, const void *initialData)
	{
		if (usage == BufferUsage::BUFFER_USAGE_STATIC && (initialSize == 0 || initialData == nullptr))
		{
			return false;
		}

		if (initialSize == 0)
		{
			return false;
		}

		this->type = type;
		this->usage = usage;

		binding = GFXtoGL::BufferBinding[static_cast<int>(type)];
		mode = GFXtoGL::BufferMode[static_cast<int>(usage)];

		glGenBuffersARB(1, &handle);
		glBindBufferARB(binding, handle);
		glBufferDataARB(binding, initialSize, (initialData != nullptr ? initialData : NULL), mode);

		size = initialSize;

		LOG_INFO_ARGS("Created a new buffer with handle %u", handle);
		return true;
	}

	bool Buffer_OpenGL::Initialize(BufferType type, BufferUsage usage, IndexFormat indexFormat, size_t initialSize, const void *initialData)
	{
		if (type != BufferType::BUFFER_INDEX)
		{
			return false;
		}

		if (Initialize(type, usage, initialSize, initialData) == false)
		{
			return false;
		}

		this->indexFormat = indexFormat;
		return true;
	}

	void Buffer_OpenGL::Destroy()
	{
		if (handle == 0)
		{
			return;
		}

		glDeleteBuffersARB(1, &handle);
		if (nameSet)
		{
			LOG_INFO_ARGS("Buffer \"%s\" has been destroyed", name);
		}
		else
		{
			LOG_INFO_ARGS("Buffer %d has been destroyed", handle);
		}

		handle = 0;
	}

	void Buffer_OpenGL::Bind() const
	{
		if (handle != 0)
		{
			glBindBufferARB(binding, handle);
		}
	}

	void Buffer_OpenGL::SetData(const void *src, size_t offset, size_t size)
	{
		if (handle != 0 && src != nullptr && size > 0 && (offset + size <= this->size))
		{
			glBindBufferARB(binding, handle);
			glBufferSubDataARB(binding, offset, size, src);
		}
	}

	void *Buffer_OpenGL::Map(BufferMapping mapping)
	{
		if (handle == 0)
		{
			return nullptr;
		}

		if (mapped)
		{
			return mappedAddress;
		}

		GLenum glMapping = GFXtoGL::BufferMapping[static_cast<int>(mapping)];

		glBindBufferARB(binding, handle);
		mappedAddress = glMapBufferARB(binding, glMapping);
		mapped = true;
		return mappedAddress;
	}

	bool Buffer_OpenGL::Unmap()
	{
		if (mapped)
		{
			glBindBufferARB(binding, handle);
			GLboolean result = glUnmapBufferARB(binding);

			mapped = false;
			mappedAddress = nullptr;

			return (result == GL_TRUE);
		}

		return false;
	}
}
