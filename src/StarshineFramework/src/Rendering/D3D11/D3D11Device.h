#pragma once
#include "Rendering/Device.h"
#include <d3d11.h>
#include <wrl.h>

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
		void SetViewportSize(const RectangleF& newSize);

	public:
		void Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil);
		void SwapBuffers();

	public:
		void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount);
		void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 baseVertexIndex, u32 indexCount);

	public:
		bool CreateBuffer(const BufferCreationData& props, std::unique_ptr<Buffer>& buffer);

		bool CreateShader(const void* vsData, size_t vsSize, const void* fsData, size_t fsSize, std::unique_ptr<Shader>& shader);
		bool CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount, const Shader* shader, std::unique_ptr<VertexDesc>& desc);

		bool UploadTexture(Graphics::Texture* texture);

		bool CreateBlendState(const BlendStateDesc& desc, std::unique_ptr<BlendState>& state);

	public:
		void SetVertexBuffer(const Buffer* buffer, const VertexDesc* desc);
		void SetIndexBuffer(const Buffer* buffer);
		void SetUniformBuffer(const Buffer* buffer, ShaderStage stage, u32 bufferIndex);
		void SetShader(const Shader* shader);
		void SetTexture(Graphics::Texture* texture, u32 slot);

		void SetBlendState(const BlendState* state);

	public:
		void QueueObjectForDeletion(Microsoft::WRL::ComPtr<ID3D11DeviceChild> object);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{ nullptr };
	};
}
