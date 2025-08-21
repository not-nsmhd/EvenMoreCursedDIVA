#pragma once
#include "common/types.h"
#include "common/color.h"
#include "common/rect.h"
#include "Types.h"
#include "Resource.h"
#include "Buffers.h"
#include "VertexDesc.h"
#include "Shader.h"
#include "Texture.h"
#include <string_view>
#include <SDL2/SDL.h>

namespace Starshine::GFX
{
	class Renderer : NonCopyable
	{
	public:
		Renderer(RendererBackendType backend);
		~Renderer() = default;

	public:
		static void CreateInstance(RendererBackendType backend);
		static void DeleteInstance();

		static Renderer* GetInstance();

	public:
		bool Initialize(SDL_Window* gameWindow);
		void Destroy();

	public:
		RendererBackendType GetType() const;

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

		Shader* LoadShaderFromXml(const u8* xmlData, size_t xmlSize);
		Shader* LoadShaderFromXml(const std::string_view filePath);

		Texture* CreateTexture(u32 width, u32 height, TextureFormat format, bool nearestFilter, bool clamp);
		Texture* LoadTexture(const u8* fileData, size_t fileSize, bool nearestFilter, bool clamp);
		Texture* LoadTexture(const std::string_view filePath, bool nearestFilter, bool clamp);

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
