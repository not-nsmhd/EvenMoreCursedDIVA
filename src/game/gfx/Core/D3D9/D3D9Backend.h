#pragma once
#include "common/types.h"
#include "gfx/Core/IBackend.h"

namespace Starshine::GFX::Core::D3D9
{
	class D3D9Backend;
	struct ResourceContext;

	class D3D9Backend : public IBackend, NonCopyable
	{
	public:
		D3D9Backend();
		~D3D9Backend();

	public:
		bool Initialize(SDL_Window* gameWindow);
		void Destroy();

	public:
		RendererBackendType GetType() const;

	public:
		ResourceContext* GetResourceContext(ResourceHandle handle);

		Common::RectangleF GetViewportSize() const;
	public:
		void Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil);
		void SwapBuffers();

	public:
		void SetBlendState(bool enable, BlendFactor srcColor, BlendFactor destColor, BlendFactor srcAlpha, BlendFactor destAlpha);
		void SetBlendOperation(BlendOperation op);

		void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount);
		void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 indexCount);

	public:
		VertexBuffer* CreateVertexBuffer(size_t size, void* initialData, bool dynamic);
		IndexBuffer* CreateIndexBuffer(size_t size, IndexFormat format, void* initialData, bool dynamic);

		VertexDesc* CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount);
		Shader* LoadShader(const u8* vsData, size_t vsSize, const u8* fsData, size_t fsSize);

		Texture* CreateTexture(u32 width, u32 height, TextureFormat format, bool nearestFilter, bool clamp);

		void DeleteResource(Resource* resource);

	public:
		void SetVertexBuffer(const VertexBuffer* buffer);
		void SetIndexBuffer(const IndexBuffer* buffer);
		void SetVertexDesc(const VertexDesc* desc);
		void SetShader(Shader* shader);
		void SetTexture(const Texture* texture, u32 slot);

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
