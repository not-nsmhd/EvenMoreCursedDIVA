#pragma once
#include "Rendering/Texture.h"
#include "OpenGLDevice.h"
#include <glad/glad.h>

namespace Starshine::Rendering::OpenGL
{
	struct Texture_OpenGL : public Texture
	{
		Texture_OpenGL(OpenGLDevice& device, u32 width, u32 height, GFX::TextureFormat format, bool dynamic);
		~Texture_OpenGL();

		u32 GetWidth() const;
		u32 GetHeight() const;
		GFX::TextureFormat GetFormat() const;

		void SetData(const void* source, u32 x, u32 y, u32 width, u32 height);

		OpenGLDevice& DeviceRef;
		GLuint Handle{};

		GLenum Filter{ GL_LINEAR };
		GLenum WrapMode{ GL_CLAMP_TO_EDGE };

		u32 Width{};
		u32 Height{};
		GFX::TextureFormat Format{ GFX::TextureFormat::Unknown };
		bool Dynamic{};
	};
}
