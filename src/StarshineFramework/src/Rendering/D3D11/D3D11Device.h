#pragma once
#include "Rendering/Device.h"
#include <d3d11.h>

namespace Starshine::Rendering::D3D11
{
	class D3D11Device : public Device
	{
	public:
		D3D11Device();
		~D3D11Device();

	public:
		ID3D11Device* GetBaseDevice();

	public:
		bool Initialize(SDL_Window* gameWindow);
		void Destroy();

	public:
		void OnWindowResize(i32 width, i32 height);

	public:
		RectangleF GetViewportSize() const;

	public:
		void Clear(ClearFlags flags, const Color & color, f32 depth, u8 stencil);
		void SwapBuffers();

	public:
		void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount);
		void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 baseVertexIndex, u32 indexCount);

	public:
		bool CreateVertexBuffer(size_t size, const void* initialData, bool dynamic, std::unique_ptr<VertexBuffer>& buffer);
		bool CreateIndexBuffer(size_t size, IndexFormat format, const void* initialData, bool dynamic, std::unique_ptr<IndexBuffer>& buffer);
		bool CreateUniformBuffer(size_t size, const void* initialData, bool dynamic, std::unique_ptr<UniformBuffer>& buffer);

		bool CreateShader(const void* vsData, size_t vsSize, const void* fsData, size_t fsSize, std::unique_ptr<Shader>& shader);
		bool CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount, const Shader* shader, std::unique_ptr<VertexDesc>& desc);

		bool CreateTexture(i32 width, i32 height, GFX::TextureFormat format, const void* initialData, std::unique_ptr<Texture>& texture);

		bool CreateBlendState(const BlendStateDesc& desc, std::unique_ptr<BlendState>& state);

	public:
		void SetVertexBuffer(const VertexBuffer* buffer, const VertexDesc* desc);
		void SetIndexBuffer(const IndexBuffer* buffer);
		void SetUniformBuffer(const UniformBuffer* buffer, ShaderStage stage, u32 bufferIndex);
		void SetShader(const Shader* shader);
		void SetTexture(const Texture* texture, u32 slot);

		void SetBlendState(const BlendState* state);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{ nullptr };
	};
}
