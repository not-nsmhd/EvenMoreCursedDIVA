#pragma once
#if 0
#include "common/types.h"
#include "gfx/new/Core/IBackend.h"

namespace Starshine::GFX::Core::D3D9
{
	struct ResourceContext;

	struct VertexBuffer_D3D9 : public VertexBuffer
	{
		ResourceContext* Context = nullptr;

		void SetData(void* source, size_t offset, size_t size);
	};

	class D3D9Backend : public IBackend, NonCopyable
	{
		friend VertexBuffer_D3D9;

	public:
		D3D9Backend();
		~D3D9Backend();

	public:
		bool Initialize(SDL_Window* gameWindow);
		void Destroy();

	public:
		RendererBackendType GetType() const;

	public:
		void Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil);
		void SwapBuffers();

	public:
		VertexBuffer* CreateVertexBuffer(size_t size, void* initialData, bool dynamic);
		void DeleteResource(Resource* resource);

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
#endif
