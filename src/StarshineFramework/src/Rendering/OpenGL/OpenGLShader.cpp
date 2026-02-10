#include "OpenGLShader.h"
#include <glm/gtc/type_ptr.hpp>

namespace Starshine::Rendering::OpenGL
{
	Shader_OpenGL::Shader_OpenGL(OpenGLDevice& device, GLuint programHandle)
		: DeviceRef(device), ProgramHandle(programHandle)
	{}

	Shader_OpenGL::~Shader_OpenGL()
	{
		glDeleteProgram(ProgramHandle);
	}

	void Shader_OpenGL::SetVertexShaderVariables(u32 index, size_t count, const f32* values)
	{
		if (ProgramHandle != 0 && values != nullptr && count > 0)
		{
			glProgramUniform4fv(ProgramHandle, index, count, values);
		}
	}

	void Shader_OpenGL::SetFragmentShaderVariables(u32 index, size_t count, const f32* values)
	{
		if (ProgramHandle != 0 && values != nullptr && count > 0)
		{
			glProgramUniform4fv(ProgramHandle, index, count, values);
		}
	}

	void Shader_OpenGL::SetVertexShaderMatrix(u32 index, const mat4& matrix)
	{
		if (ProgramHandle != 0)
		{
			mat4 transposed = glm::transpose(matrix);
			auto valuePtr = glm::value_ptr(transposed);

			glProgramUniformMatrix4fv(ProgramHandle, index, 1, GL_FALSE, valuePtr);
		}
	}

	void Shader_OpenGL::SetFragmentShaderMatrix(u32 index, const mat4& matrix)
	{
		if (ProgramHandle != 0)
		{
			mat4 transposed = glm::transpose(matrix);
			auto valuePtr = glm::value_ptr(transposed);

			glProgramUniformMatrix4fv(ProgramHandle, index, 1, GL_FALSE, valuePtr);
		}
	}
}
