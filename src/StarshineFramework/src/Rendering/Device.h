#pragma once
#include "Common/Types.h"
#include "Common/Rect.h"
#include "Common/Color.h"
#include "Rendering/Types.h"
#include "Rendering/Buffers.h"
#include "Rendering/VertexDesc.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"
#include "Rendering/State.h"
#include "Graphics/Texture.h"
#include <memory>
#include <SDL2/SDL_video.h>

namespace Starshine::Rendering
{
	class Device : public NonCopyable
	{
	public:
		Device() = default;
		~Device() = default;

	public:
		virtual bool Initialize(SDL_Window* gameWindow) = 0;
		virtual void Destroy() = 0;
		virtual void ReportExistingObjects() = 0;

	public:
		virtual void OnWindowResize(i32 width, i32 height) = 0;

	public:
		virtual RectangleF GetViewportSize() const = 0;
		virtual void SetViewportSize(const RectangleF& newSize) = 0;

	public:
		virtual void Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil) = 0;
		virtual void SwapBuffers() = 0;

	public:
		virtual void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount) = 0;
		virtual void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 baseVertexIndex, u32 indexCount) = 0;

	public:
		virtual bool CreateBuffer(const BufferCreationData& props, std::unique_ptr<Buffer>& buffer) = 0;

		virtual bool CreateShader(const void* vsData, size_t vsSize, const void* fsData, size_t fsSize, std::unique_ptr<Shader>& shader) = 0;
		virtual bool CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount, const Shader* shader, std::unique_ptr<VertexDesc>& desc) = 0;
		 
		virtual bool UploadTexture(Graphics::Texture* texture) = 0;

		virtual bool CreateBlendState(const BlendStateDesc& desc, std::unique_ptr<BlendState>& state) = 0;

	public:
		virtual void SetVertexBuffer(const Buffer* buffer, const VertexDesc* desc) = 0;
		virtual void SetIndexBuffer(const Buffer* buffer) = 0;
		virtual void SetUniformBuffer(const Buffer* buffer, ShaderStage stage, u32 bufferIndex) = 0;
		virtual void SetShader(const Shader* shader) = 0;
		virtual void SetTexture(Graphics::Texture* texture, u32 slot) = 0;

		virtual void SetBlendState(const BlendState* state) = 0;

	public:
		bool ResizeViewportOnWindowResize{ true };
	};

	bool InitializeDevice(SDL_Window* sdlWindow, DeviceType type);
	void DestroyDevice();
	Device* GetDevice();
	DeviceType GetDeviceType();
}
