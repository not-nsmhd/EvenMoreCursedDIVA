#include <SDL2/SDL.h>
#include "opengl_defs.h"
#include "opengl_texture.h"

namespace GFX::LowLevel::OpenGL_ARB
{
	namespace GFXtoGL
	{
		GLenum InternalFormat[] =
			{
				GL_RGBA8,	// TEXFORMAT_RGBA8
				GL_RGB5,	// TEXFORMAT_RGB565
				GL_RGB5_A1, // TEXFORMAT_RGB5_A1
				GL_RGBA4,	// TEXFORMAT_RGBA4
				1			// TEXFORMAT_R8
		};

		GLenum PixelFormat[] =
			{
				GL_RGBA, // TEXFORMAT_RGBA8
				GL_RGB,	 // TEXFORMAT_RGB565
				GL_RGBA, // TEXFORMAT_RGB5_A1
				GL_RGBA, // TEXFORMAT_RGBA4
				GL_RED	 // TEXFORMAT_R8
		};
	}

	TextureFormat Texture_OpenGL::GetFormat() const
	{
		return format;
	}

	u32 Texture_OpenGL::GetWidth() const
	{
		return width;
	}

	u32 Texture_OpenGL::GetHeight() const
	{
		return height;
	}

	GLuint Texture_OpenGL::GetHandle() const
	{
		return handle;
	}

	bool Texture_OpenGL::Initialize(int width, int height, TextureFormat format, u32 flags)
	{
		if (width == 0 || height == 0 || format == TextureFormat::TEXFORMAT_UNKNOWN)
		{
			return false;
		}

		this->width = width;
		this->height = height;
		this->format = format;

		pixelFormat = GFXtoGL::PixelFormat[static_cast<int>(format)];
		internalFormat = GFXtoGL::InternalFormat[static_cast<int>(format)];

		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_2D, handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, NULL);

		LOG_INFO_ARGS("Created a new %dx%d texture (handle: %u)", width, height, handle);

		return true;
	}

	void Texture_OpenGL::Destroy()
	{
		if (handle != 0)
		{
			glDeleteTextures(1, &handle);
			
			if (nameSet)
			{
				LOG_INFO_ARGS("Destroyed texture \"%s\" (%dx%d)", name, width, height);
			}
			else
			{
				LOG_INFO_ARGS("Destroyed a %dx%d texture (handle: %u)", width, height, handle);
			}

			handle = 0;
		}
	}

	void Texture_OpenGL::Bind(u32 unit) const
	{
		if (handle == 0)
		{
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(GL_TEXTURE_2D, 0);
			return;
		}

		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, handle);
	}

	void Texture_OpenGL::SetData(const void *data)
	{
		if (handle != 0 && data != nullptr)
		{
			glBindTexture(GL_TEXTURE_2D, handle);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, pixelFormat, GL_UNSIGNED_BYTE, data);
		}
	}

	void Texture_OpenGL::SetData(const void *data, u32 x, u32 y, u32 width, u32 height)
	{
		if (handle != 0 && data != nullptr)
		{
			glBindTexture(GL_TEXTURE_2D, handle);
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, pixelFormat, GL_UNSIGNED_BYTE, data);
		}
	}
}
