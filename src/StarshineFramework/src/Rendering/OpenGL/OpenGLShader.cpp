#include "OpenGLShader.h"
#include <glm/gtc/type_ptr.hpp>

namespace Starshine::Rendering::OpenGL
{
	Shader_D3D9::Shader_D3D9(OpenGLDevice& device, GLuint vsHandle, GLuint fsHandle)
		: DeviceRef(device), VertexProgramHandle(vsHandle), FragmentProgramHandle(fsHandle)
	{}

	Shader_D3D9::~Shader_D3D9()
	{
		glDeleteProgramsARB(1, &VertexProgramHandle);
		glDeleteProgramsARB(1, &FragmentProgramHandle);
	}

	void Shader_D3D9::SetVertexShaderVariables(u32 index, size_t count, const f32* values)
	{
		if (VertexProgramHandle != 0 && FragmentProgramHandle != 0 && values != nullptr && count > 0)
		{
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, VertexProgramHandle);
			for (size_t i = 0; i < count; i++)
			{
				glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, i, &values[i * 4]);
			}
		}
	}

	void Shader_D3D9::SetFragmentShaderVariables(u32 index, size_t count, const f32* values)
	{
		if (VertexProgramHandle != 0 && FragmentProgramHandle != 0 && values != nullptr && count > 0)
		{
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragmentProgramHandle);
			for (size_t i = 0; i < count; i++)
			{
				glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, i, &values[i * 4]);
			}
		}
	}

	void Shader_D3D9::SetVertexShaderMatrix(u32 index, const mat4& matrix)
	{
		if (VertexProgramHandle != 0 && FragmentProgramHandle != 0)
		{
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, VertexProgramHandle);

			mat4 transposed = glm::transpose(matrix);
			auto valuePtr = glm::value_ptr(transposed);

			glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index + 0, &valuePtr[0]);
			glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index + 1, &valuePtr[4]);
			glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index + 2, &valuePtr[8]);
			glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index + 3, &valuePtr[12]);
		}
	}

	void Shader_D3D9::SetFragmentShaderMatrix(u32 index, const mat4& matrix)
	{
		if (VertexProgramHandle != 0 && FragmentProgramHandle != 0)
		{
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragmentProgramHandle);

			mat4 transposed = glm::transpose(matrix);
			auto valuePtr = glm::value_ptr(transposed);

			glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index + 0, &valuePtr[0]);
			glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index + 1, &valuePtr[4]);
			glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index + 2, &valuePtr[8]);
			glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index + 3, &valuePtr[12]);
		}
	}
}
