#pragma once
#include "common/types.h"
#include "common/color.h"
#include "gfx/new/Types.h"
#include "gfx/new/Buffers.h"
#include "gfx/new/VertexDesc.h"
#include "gfx/new/Shader.h"
#include "gfx/new/Texture.h"
#include <SDL2/SDL.h>

namespace Starshine::GFX::Core
{
	class IBackend
	{
	public:
		virtual ~IBackend() = default;

	public:
		virtual bool Initialize(SDL_Window* gameWindow) = 0;
		virtual void Destroy() = 0;

	public:
		virtual RendererBackendType GetType() const = 0;

	public:
		virtual void Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil) = 0;
		virtual void SwapBuffers() = 0;

		virtual void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount) = 0;
		virtual void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 indexCount) = 0;

	public:
		virtual VertexBuffer* CreateVertexBuffer(size_t size, void* initialData, bool dynamic) = 0;
		virtual IndexBuffer* CreateIndexBuffer(size_t size, IndexFormat format, void* initialData, bool dynamic) = 0;
		virtual VertexDesc* CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount) = 0;
		virtual Shader* LoadShader(const u8* vsData, size_t vsSize, const u8* fsData, size_t fsSize) = 0;
		virtual Texture* CreateTexture(u32 width, u32 height, TextureFormat format, bool nearestFilter, bool clamp) = 0;

	public:
		virtual void SetVertexBuffer(const VertexBuffer* buffer) = 0;
		virtual void SetIndexBuffer(const IndexBuffer* buffer) = 0;
		virtual void SetVertexDesc(const VertexDesc* desc) = 0;
		virtual void SetShader(const Shader* shader) = 0;
		virtual void SetTexture(const Texture* texture, u32 slot) = 0;

		virtual void DeleteResource(Resource* resource) = 0;
	};
}
