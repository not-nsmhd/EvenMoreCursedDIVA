#pragma once
#include "Rendering/Types.h"
#include <glad/glad.h>

namespace Starshine::Rendering::OpenGL
{
	namespace ConversionTables
	{
		constexpr std::array<GLenum, EnumCount<PrimitiveType>()> GLPrimitiveTypes =
		{
			GL_POINTS,
			GL_LINES,
			GL_LINE_STRIP,
			GL_TRIANGLES,
			GL_TRIANGLE_STRIP
		};

		constexpr std::array<GLenum, EnumCount<IndexFormat>()> GLIndexFormats =
		{
			GL_UNSIGNED_SHORT,
			GL_UNSIGNED_INT
		};

		constexpr std::array<GLenum, EnumCount<BlendFactor>()> GLBlendFactors =
		{
			GL_ZERO,
			GL_ONE,
			GL_SRC_COLOR,
			GL_ONE_MINUS_SRC_COLOR,
			GL_DST_COLOR,
			GL_ONE_MINUS_DST_COLOR,
			GL_SRC_ALPHA,
			GL_ONE_MINUS_SRC_ALPHA,
			GL_DST_ALPHA,
			GL_ONE_MINUS_DST_ALPHA
		};

		constexpr std::array<GLenum, EnumCount<BlendOperation>()> GLBlendOperations =
		{
			GL_FUNC_ADD,
			GL_FUNC_SUBTRACT,
			GL_FUNC_REVERSE_SUBTRACT,
			GL_MIN,
			GL_MAX
		};

		constexpr std::array<GLenum, EnumCount<PolygonOrientation>()> GLPolygonOrientation =
		{
			GL_CW,
			GL_CCW
		};

		constexpr std::array<GLenum, EnumCount<Face>()> GLFace =
		{
			GL_FRONT,
			GL_BACK,
			GL_FRONT_AND_BACK
		};

		constexpr std::array<GLenum, EnumCount<GFX::TextureFormat>()> GLTextureDataFormats =
		{
			GL_RGBA,
			2, // https://docs.gl/gl2/glTexImage2D
			GL_RED,

			0,
			0,
			0
		};

		constexpr std::array<GLenum, EnumCount<GFX::TextureFormat>()> GLTextureDisplayFormats =
		{
			GL_RGBA,
			GL_LUMINANCE_ALPHA,
			GL_RED,

			GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
		};
	}
}
