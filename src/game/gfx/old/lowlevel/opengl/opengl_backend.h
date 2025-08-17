#pragma once
#include "../backend.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace OpenGL_ARB
		{
			class Backend_OpenGL : public Backend
			{
			public:
				Backend_OpenGL() { type = BackendType::OpenGL_ARB; };

				bool Initialize(SDL_Window* window);
				void Destroy();

				void Clear(u32 flags, Color color, float depth, u8 stencil);
				void SwapBuffers();

				void Begin();
				void End();

				void GetViewportSize(float* x, float* y, float* width, float* height);
				void ResizeMainRenderTarget(u32 newWidth, u32 newHeight);

				void SetBlendState(const BlendState* state);

				Buffer* CreateVertexBuffer(BufferUsage usage, const void* initialData, u32 size);
				Buffer* CreateIndexBuffer(BufferUsage usage, IndexFormat format, const void* initialData, u32 size);
				void DestroyBuffer(Buffer* buffer);

				void BindVertexBuffer(Buffer* buffer);
				void BindIndexBuffer(Buffer* buffer);

				void SetBufferData(Buffer* buffer, const void* src, u32 offset, u32 size);
				void* MapBuffer(Buffer* buffer, BufferMapping mapMode);
				void UnmapBuffer(Buffer* buffer);

				Shader* CreateShader(const u8* vsSource, u32 vsSourceLength, const u8* fsSource, u32 fsSourceLength);
				void DestroyShader(Shader* shader);

				void BindShader(Shader* shader);

				void SetShaderMatrix(u32 index, const float* matrix);
				void SetShaderMatrix(u32 index, const mat4* matrix);

				void SetShaderVector4(u32 index, vec4 matrix);

				VertexDescription* CreateVertexDescription(const VertexAttribute* attribs, u32 attribCount, u32 stride, const Shader* shader);
				void DestroyVertexDescription(VertexDescription* desc);

				void SetVertexDescription(VertexDescription* desc);

				Texture* CreateTexture(u32 width, u32 height, TextureFormat format, u32 flags);
				void DestroyTexture(Texture* texture);

				void BindTexture(Texture* texture, u32 unit);

				void SetTextureData(Texture* texture, const void* data);
				void SetTextureData(Texture* texture, const void* data, u32 x, u32 y, u32 width, u32 height);

				void DrawArrays(PrimitiveType type, i32 firstVertex, i32 vertexCount);
				void DrawIndexed(PrimitiveType type, i32 vertexCount, i32 firstIndex, i32 indexCount);
			private:
				SDL_GLContext glContext = NULL;

				bool indexBufferSet = false;
				IndexFormat currentIndexFormat = IndexFormat::INDEX_16BIT;
			};
		}
	}
}
