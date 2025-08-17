#pragma once
#include "common/types.h"
#include "gfx/Core/IBackend.h"
#include "OpenGLVertexDesc.h"
#include <vector>

namespace Starshine::GFX::Core::OpenGL
{
	class OpenGLBackend;
	struct ResourceContext;

	struct VertexBuffer_OpenGL : public VertexBuffer
	{
	public:
		VertexBuffer_OpenGL(ResourceHandle handle, size_t size, bool dynamic) 
			: VertexBuffer(handle), Size(size), Dynamic(dynamic) {}

		OpenGLBackend* Backend = nullptr;

		size_t Size = 0;
		bool Dynamic = false;

		void SetData(void* source, size_t offset, size_t size);
	};

	struct IndexBuffer_OpenGL : public IndexBuffer
	{
	public:
		IndexBuffer_OpenGL(ResourceHandle handle, size_t size, IndexFormat format, bool dynamic)
			: IndexBuffer(handle), Size(size), Format(format), Dynamic(dynamic) {}

		OpenGLBackend* Backend = nullptr;

		size_t Size = 0;
		bool Dynamic = false;
		IndexFormat Format = {};

		void SetData(void* source, size_t offset, size_t size);
	};

	struct Shader_OpenGL : public Shader
	{
	public:
		Shader_OpenGL(ResourceHandle handle) : Shader(handle) {}

		OpenGLBackend* Backend = nullptr;

		ResourceHandle VertexHandle = InvalidResourceHandle;
		ResourceHandle FragmentHandle = InvalidResourceHandle;

		std::vector<ShaderVariable> Variables;
		bool UpdateVariables = false;

		void AddVariable(ShaderVariable& variable);

		ShaderVariableIndex GetVariableIndex(std::string_view name);
		void SetVariableValue(ShaderVariableIndex varIndex, void* value);
	};

	struct VertexDesc_OpenGL : public VertexDesc
	{
	public:
		VertexDesc_OpenGL(ResourceHandle handle, const VertexAttrib* attribs, size_t attribCount) 
			: VertexDesc(handle), GLAttribs(attribCount) {}

		std::vector<VertexAttrib_OpenGL> GLAttribs;
	};

	struct Texture_OpenGL : public Texture
	{
	public:
		Texture_OpenGL(ResourceHandle handle, u32 width, u32 height, TextureFormat format, bool clamp, bool nearestFilter)
			: Texture(handle),
			Width(width), Height(height), Format(format), Clamp(clamp), NearestFilter(nearestFilter) {}

		OpenGLBackend* Backend = nullptr;

		u32 Width = 0;
		u32 Height = 0;
		TextureFormat Format{};

		bool NearestFilter = false;
		bool Clamp = false;

		u32 GetWidth() const;
		u32 GetHeight() const;

		void SetData(u32 x, u32 y, u32 width, u32 height, const void* data);
	};

	class OpenGLBackend : public IBackend, NonCopyable
	{
	public:
		OpenGLBackend();
		~OpenGLBackend();

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

		void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount);
		void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 indexCount);

	public:
		VertexBuffer* CreateVertexBuffer(size_t size, void* initialData, bool dynamic);
		IndexBuffer* CreateIndexBuffer(size_t size, IndexFormat format, void* initialData, bool dynamic);

		// NOTE: Vertex descriptions do not have their own contexts in OpenGL
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
