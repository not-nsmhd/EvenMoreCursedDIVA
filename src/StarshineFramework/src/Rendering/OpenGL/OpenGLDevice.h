#pragma once
#include "Rendering/Device.h"

namespace Starshine::Rendering::OpenGL
{
	class OpenGLDevice : public Device
	{
	public:
		OpenGLDevice();
		~OpenGLDevice();

	public:
		bool Initialize(SDL_Window* gameWindow);
		void Destroy();

	public:
		RectangleF GetViewportSize() const;
	public:
		void Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil);
		void SwapBuffers();

	public:
		void SetBlendState(bool enable, BlendFactor srcColor, BlendFactor destColor, BlendFactor srcAlpha, BlendFactor destAlpha);
		void SetBlendOperation(BlendOperation op);

		void SetFaceCullingState(bool enable, PolygonOrientation backFaceOrientation);

		void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount);
		void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 vertexCount, u32 indexCount);

	public:
		std::unique_ptr<VertexBuffer> CreateVertexBuffer(size_t size, const void* initialData, bool dynamic);
		std::unique_ptr<IndexBuffer> CreateIndexBuffer(size_t size, IndexFormat format, const void* initialData, bool dynamic);
		std::unique_ptr<VertexDesc> CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount);

		std::unique_ptr<Shader> LoadShader(const void* vsData, size_t vsSize, const void* fsData, size_t fsSize);

		std::unique_ptr<Texture> CreateTexture(u32 width, u32 height, GFX::TextureFormat format, bool nearestFilter, bool repeat);

	public:
		void SetVertexBuffer(const VertexBuffer* buffer, const VertexDesc* desc);
		void SetIndexBuffer(const IndexBuffer* buffer);
		void SetShader(const Shader* shader);
		void SetTexture(const Texture* texture, u32 slot);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl = nullptr;
	};
}
