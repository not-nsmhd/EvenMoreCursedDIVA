#pragma once
#include <glad/glad.h>
#include "../texture.h"

namespace GFX::LowLevel::OpenGL_ARB
{
	class Texture_OpenGL : public Texture
	{
	public:
		Texture_OpenGL() {}

		u32 GetWidth() const;
		u32 GetHeight() const;
		TextureFormat GetFormat() const;

		GLuint GetHandle() const;

		bool Initialize(int width, int height, TextureFormat format, u32 flags);
		void Destroy();

		void Bind(u32 unit) const;

		void SetData(const void *data);
		void SetData(const void *data, u32 x, u32 y, u32 width, u32 height);
	private:
		GLuint handle = 0;

		GLenum pixelFormat = 0;
		GLenum internalFormat = 0;
	};
}
