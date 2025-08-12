#include <SDL2/SDL.h>
#include "opengl_shader.h"
#include "opengl_defs.h"

namespace GFX::LowLevel::OpenGL_ARB
{
	GLuint Shader_OpenGL::GetVertexHandle() const
	{
		return vpHandle;
	}

	GLuint Shader_OpenGL::GetFragmentHandle() const
	{
		return fpHandle;
	}

	bool Shader_OpenGL::Initialize(const u8 *vsSource, size_t vsSourceSize, const u8 *fsSource, size_t fsSourceSize)
	{
		if (vsSource == nullptr || vsSourceSize == 0 ||
			fsSource == nullptr || fsSourceSize == 0)
		{
			return false;
		}

		glGenProgramsARB(1, &vpHandle);
		glGenProgramsARB(1, &fpHandle);

		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vpHandle);
		glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, vsSourceSize, vsSource);

		GLint errorPos = -1;
		const GLubyte *errorString = nullptr;

		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (errorPos != -1)
		{
			errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LOG_ERROR_ARGS("Failed to assemble vertex program. Error: %s", errorString);
		}

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fpHandle);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, fsSourceSize, fsSource);

		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (errorPos != -1)
		{
			errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LOG_ERROR_ARGS("Failed to assemble fragment program. Error: %s", errorString);
		}

		LOG_INFO_ARGS("Created a new shader program (vp: %u, fp: %u)", vpHandle, fpHandle);
		return true;
	}

	void Shader_OpenGL::Destroy()
	{
		glDeleteProgramsARB(1, &vpHandle);
		glDeleteProgramsARB(1, &fpHandle);
		
		if (nameSet)
		{
			LOG_INFO_ARGS("Shader program \"%s\" has been destroyed", name);
		}
		else
		{
			LOG_INFO_ARGS("Shader program (vp: %u, fp: %u) has been destroyed", vpHandle, fpHandle);
		}
	}

	void Shader_OpenGL::Bind() const
	{
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vpHandle);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fpHandle);
	}
}
