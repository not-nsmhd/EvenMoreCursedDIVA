#include "OpenGLTexture.h"
#include "OpenGLCommon.h"

namespace Starshine::Rendering::OpenGL
{
	Texture_D3D9::Texture_D3D9(OpenGLDevice& device, u32 width, u32 height, GFX::TextureFormat format, bool dynamic) :
		DeviceRef(device), Width(width), Height(height), Format(format), Dynamic(dynamic)
	{
	}

	Texture_D3D9::~Texture_D3D9()
	{
		glDeleteTextures(1, &Handle);
		Handle = 0;
	}

	u32 Texture_D3D9::GetWidth() const
	{
		return Width;
	}

	u32 Texture_D3D9::GetHeight() const
	{
		return Height;
	}

	GFX::TextureFormat Texture_D3D9::GetFormat() const
	{
		return Format;
	}

	void Texture_D3D9::SetData(const void* source, u32 x, u32 y, u32 width, u32 height)
	{
		if (Handle != 0 && source != nullptr)
		{
			if (x + width > Width || y + height > Height)
			{
				return;
			}

			GLenum pixelFormat = ConversionTables::GLTextureDisplayFormats[static_cast<size_t>(Format)];

			glBindTexture(GL_TEXTURE_2D, Handle);
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, pixelFormat, GL_UNSIGNED_BYTE, source);
		}
	}
}
