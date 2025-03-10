#pragma once
#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include <d3d9.h>
#include "../backend.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace D3D9
		{
			class Backend_D3D9 : public Backend
			{
			public:
				Backend_D3D9() { type = BackendType::BACKEND_D3D9; };

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

				void BindVertexBuffer(const Buffer* buffer);
				void BindIndexBuffer(const Buffer* buffer);

				void SetBufferData(Buffer* buffer, const void* src, u32 offset, u32 size);
				void* MapBuffer(Buffer* buffer, BufferMapping mapMode);
				void UnmapBuffer(Buffer* buffer);

				Shader* CreateShader(const u8* vsSource, u32 vsSourceLength, const u8* fsSource, u32 fsSourceLength);
				void DestroyShader(Shader* shader);

				void BindShader(const Shader* shader);

				void SetShaderMatrix(u32 index, const float* matrix);

				VertexDescription* CreateVertexDescription(const VertexAttribute* attribs, u32 attribCount, u32 stride, const Shader* shader);
				void DestroyVertexDescription(VertexDescription* desc);

				void SetVertexDescription(const VertexDescription* desc);

				Texture* CreateTexture(u32 width, u32 height, TextureFormat format, u32 flags);
				void DestroyTexture(Texture* texture);

				void BindTexture(const Texture* texture, u32 unit);

				void SetTextureData(Texture* texture, const void* data);
				void SetTextureData(Texture* texture, const void* data, u32 x, u32 y, u32 width, u32 height);

				void DrawArrays(PrimitiveType type, i32 firstVertex, i32 vertexCount);
				void DrawIndexed(PrimitiveType type, i32 vertexCount, i32 firstIndex, i32 indexCount);
			private:
				bool vertexDescSet = false;
				UINT currentVertexStride = 0;

				bool vertexBufferSet = false;

				bool indexBufferSet = false;
				IndexFormat currentIndexFormat = IndexFormat::INDEX_16BIT;

				IDirect3D9* d3d9;
				IDirect3DDevice9* d3d9Device;
			};
		}
	}
}
#endif
#endif
