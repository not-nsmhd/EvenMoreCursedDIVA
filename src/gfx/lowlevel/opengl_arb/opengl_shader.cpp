#include <SDL2/SDL.h>
#include "opengl_shader.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace OpenGL_ARB
		{
			const char* Shader_OpenGL::GetDebugName() const
			{
				return debugName;
			}

			void Shader_OpenGL::SetDebugName(const char* name)
			{
				size_t nameLength = SDL_strlen(name);
				nameLength = SDL_min(nameLength, 128);

				debugName = new char[nameLength + 1];
				SDL_memcpy(debugName, name, nameLength);
				debugName[nameLength] = '\0';
			}
			
			GLuint Shader_OpenGL::GetVertexHandle() const
			{
				return vpHandle;
			}
			
			GLuint Shader_OpenGL::GetFragmentHandle() const
			{
				return fpHandle;
			}

			bool Shader_OpenGL::Initialize(const u8* vsSource, size_t vsSourceSize, const u8* fsSource, size_t fsSourceSize)
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
				const GLubyte* errorString = nullptr;

				glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
				if (errorPos != -1)
				{
					errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[GFX::OpenGL]: Failed to assemble vertex program.\n%s", errorString);
				}

				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fpHandle);
				glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, fsSourceSize, fsSource);

				glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
				if (errorPos != -1)
				{
					errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[GFX::OpenGL]: Failed to assemble fragment program.\n%s", errorString);
				}

				return true;
			}

			void Shader_OpenGL::Destroy()
			{
				glDeleteProgramsARB(1, &vpHandle);
				glDeleteProgramsARB(1, &fpHandle);
			}
			
			void Shader_OpenGL::Bind() const
			{
				glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vpHandle);
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fpHandle);
			}
		}
	}
}
