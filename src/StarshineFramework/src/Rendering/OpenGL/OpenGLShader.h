#pragma once
#include "Rendering/Shader.h"
#include "OpenGLDevice.h"
#include <glad/glad.h>

namespace Starshine::Rendering::OpenGL
{
	struct Shader_OpenGL : public Shader
	{
		Shader_OpenGL(OpenGLDevice& device, GLuint vsHandle, GLuint fsHandle);
		~Shader_OpenGL();

		void SetVertexShaderVariables(u32 index, size_t count, const f32* values);
		void SetFragmentShaderVariables(u32 index, size_t count, const f32* values);

		void SetVertexShaderMatrix(u32 index, const mat4& matrix);
		void SetFragmentShaderMatrix(u32 index, const mat4& matrix);

		OpenGLDevice& DeviceRef;

		GLuint VertexProgramHandle{};
		GLuint FragmentProgramHandle{};
	};
}
