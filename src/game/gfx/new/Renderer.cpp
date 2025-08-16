#include "Core/OpenGL/OpenGLBackend.h"
#include "Core/D3D9/D3D9Backend.h"
#include "Renderer.h"
#include "util/logging.h"

namespace Starshine::GFX
{
	using namespace Logging;
	using namespace Core;
	using OpenGLBackend = Core::OpenGL::OpenGLBackend;
	//using D3D9Backend = Core::D3D9::D3D9Backend;

	constexpr const char* LogName = "Starshine::GFX";

	Renderer* RendererInstance = nullptr;

	struct Renderer::Impl
	{
		RendererBackendType CurrentBackendType{};
		IBackend* CurrentBackend = nullptr;

		void SetBackendType(RendererBackendType type)
		{
			if (CurrentBackend == nullptr)
			{
				CurrentBackendType = type;

				switch (type)
				{
				case RendererBackendType::OpenGL:
				default:
					CurrentBackend = new OpenGLBackend();
					break;
				case RendererBackendType::D3D9:
#if 0
					CurrentBackend = new D3D9Backend();
#else
					CurrentBackendType = RendererBackendType::OpenGL;
					CurrentBackend = new OpenGLBackend();
#endif
					break;
				}
			}
		}

		bool Initialize(SDL_Window* gameWindow)
		{
			LogInfo(LogName, "Backend: %s", RendererBackendTypeNames[static_cast<size_t>(CurrentBackendType)].data());

			CurrentBackend->Initialize(gameWindow);
			return true;
		}

		void Destroy()
		{
			CurrentBackend->Destroy();
			delete CurrentBackend;
			CurrentBackend = nullptr;
		}

		void Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil)
		{
			CurrentBackend->Clear(flags, color, depth, stencil);
		}
		
		void SwapBuffers()
		{
			CurrentBackend->SwapBuffers();
		}
	};

	Renderer::Renderer(RendererBackendType backend) : impl(new Impl())
	{
		impl->SetBackendType(backend);
	}

	void Renderer::CreateInstance(RendererBackendType backend)
	{
		if (RendererInstance == nullptr)
		{
			RendererInstance = new Renderer(backend);
		}
	}

	void Renderer::DeleteInstance()
	{
		if (RendererInstance != nullptr)
		{
			delete RendererInstance;
		}
	}

	Renderer* Renderer::GetInstance()
	{
		assert(RendererInstance != nullptr);
		return RendererInstance;
	}

	bool Renderer::Initialize(SDL_Window* gameWindow)
	{
		return impl->Initialize(gameWindow);
	}

	void Renderer::Destroy()
	{
		impl->Destroy();
	}

	RendererBackendType Renderer::GetType() const
	{
		return impl->CurrentBackend->GetType();
	}

	void Renderer::Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil)
	{
		impl->Clear(flags, color, depth, stencil);
	}

	void Renderer::SwapBuffers()
	{
		impl->SwapBuffers();
	}

	void Renderer::DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
	{
		impl->CurrentBackend->DrawArrays(type, firstVertex, vertexCount);
	}

	void Renderer::DrawIndexed(PrimitiveType type, u32 firstIndex, u32 indexCount)
	{
		impl->CurrentBackend->DrawIndexed(type, firstIndex, indexCount);
	}

	VertexBuffer* Renderer::CreateVertexBuffer(size_t size, void* initialData, bool dynamic)
	{
		return impl->CurrentBackend->CreateVertexBuffer(size, initialData, dynamic);
	}

	IndexBuffer* Renderer::CreateIndexBuffer(size_t size, IndexFormat format, void* initialData, bool dynamic)
	{
		return impl->CurrentBackend->CreateIndexBuffer(size, format, initialData, dynamic);
	}

	VertexDesc* Renderer::CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount)
	{
		return impl->CurrentBackend->CreateVertexDesc(attribs, attribCount);
	}

	Shader* Renderer::LoadShader(const u8* vsData, size_t vsSize, const u8* fsData, size_t fsSize)
	{
		return impl->CurrentBackend->LoadShader(vsData, vsSize, fsData, fsSize);
	}

	Shader* Renderer::LoadShaderFromXml(const u8* xmlData, size_t xmlSize)
	{
		return impl->CurrentBackend->LoadShaderFromXml(xmlData, xmlSize);
	}

	void Renderer::DeleteResource(Resource* resource)
	{
		if (resource == nullptr)
		{
			return;
		}
		impl->CurrentBackend->DeleteResource(resource);
	}

	void Renderer::SetVertexBuffer(const VertexBuffer* buffer)
	{
		impl->CurrentBackend->SetVertexBuffer(buffer);
	}

	void Renderer::SetIndexBuffer(const IndexBuffer* buffer)
	{
		impl->CurrentBackend->SetIndexBuffer(buffer);
	}

	void Renderer::SetVertexDesc(const VertexDesc* desc)
	{
		impl->CurrentBackend->SetVertexDesc(desc);
	}

	void Renderer::SetShader(const Shader* shader)
	{
		impl->CurrentBackend->SetShader(shader);
	}
}
