#pragma once
#include "Common/Types.h"
#include "Common/Rect.h"
#include "Common/Color.h"
#include "Rendering/Types.h"
#include "Rendering/Buffers.h"
#include "Rendering/VertexDesc.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"
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

	public:
		virtual RectangleF GetViewportSize() const = 0;

	public:
		virtual void Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil) = 0;
		virtual void SwapBuffers() = 0;

	public:
		virtual void SetBlendState(bool enable, BlendFactor srcColor, BlendFactor destColor, BlendFactor srcAlpha, BlendFactor destAlpha) = 0;
		virtual void SetBlendOperation(BlendOperation op) = 0;

		virtual void SetFaceCullingState(bool enable, PolygonOrientation frontFaceOrientation, Face facesToCull) = 0;

		virtual void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount) = 0;
		virtual void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 indexCount) = 0;

	public:
		virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(size_t size, const void* initialData, bool dynamic) = 0;
		virtual std::unique_ptr<IndexBuffer> CreateIndexBuffer(size_t size, IndexFormat format, const void* initialData, bool dynamic) = 0;
		virtual std::unique_ptr<VertexDesc> CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount) = 0;

		virtual std::unique_ptr<Shader> LoadShader(const void* vsData, size_t vsSize, const void* fsData, size_t fsSize) = 0;
		 
		virtual std::unique_ptr<Texture> CreateTexture(u32 width, u32 height, GFX::TextureFormat format, bool nearestFilter, bool repeat) = 0;

	public:
		virtual void SetVertexBuffer(const VertexBuffer* buffer, const VertexDesc* desc) = 0;
		virtual void SetIndexBuffer(const IndexBuffer* buffer) = 0;
		virtual void SetShader(const Shader* shader) = 0;
		virtual void SetTexture(const Texture* texture, u32 slot) = 0;
	};

	bool InitializeDevice(SDL_Window* sdlWindow, DeviceType type);
	void DestroyDevice();
	Device* GetDevice();
	DeviceType GetDeviceType();
}
