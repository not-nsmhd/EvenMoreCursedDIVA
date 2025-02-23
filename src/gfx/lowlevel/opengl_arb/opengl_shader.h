#pragma once
#include <glad/glad.h>
#include "../shader.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace OpenGL_ARB
		{
			class Shader_OpenGL : public Shader
			{
			public:
				Shader_OpenGL() {}

				const char* GetDebugName() const;
				void SetDebugName(const char* name);

				GLuint GetVertexHandle() const;
				GLuint GetFragmentHandle() const;

				bool Initialize(const u8* vsSource, size_t vsSourceSize, const u8* fsSource, size_t fsSourceSize);
				void Destroy();

				void Bind() const;
			private:
				GLuint vpHandle = 0;
				GLuint fpHandle = 0;
			};
		}
	}
}
