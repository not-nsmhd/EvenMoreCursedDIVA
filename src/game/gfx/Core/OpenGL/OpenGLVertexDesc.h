#pragma once
#include "gfx/VertexDesc.h"
#include <array>
#include <glad/glad.h>

namespace Starshine::GFX::Core::OpenGL
{
	namespace ConversionTables
	{
		constexpr std::array<GLenum, EnumCount<VertexAttribFormat>()> GLVertexAttribFormat =
		{
			GL_BYTE,
			GL_UNSIGNED_BYTE,
			GL_SHORT,
			GL_UNSIGNED_SHORT,
			GL_INT,
			GL_UNSIGNED_INT,
			GL_FLOAT
		};
	}

	struct VertexAttrib_OpenGL
	{
		VertexAttribType Type{};
		u32 Index;

		GLenum Format{};
		u32 Components{};

		GLsizei VertexSize;
		GLsizei Offset;
	};
}
