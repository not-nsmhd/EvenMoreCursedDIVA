#include "Renderer.h"
#include "Core/OpenGL/Backend.h"
#include "util/logging.h"

namespace Starshine::GFX
{
	using namespace Logging;
	using namespace Core;
	using OpenGLBackend = Core::OpenGL::Backend;

	constexpr const char* LogName = "Starshine::GFX";

	Renderer* RendererInstance = nullptr;

	struct Renderer::Impl
	{
		IBackend* CurrentBackend = nullptr;

		void SetBackendType(RendererBackendType type)
		{
			if (CurrentBackend == nullptr)
			{
				switch (type)
				{
				case RendererBackendType::OpenGL:
				default:
					CurrentBackend = new OpenGLBackend();
					break;
				}
			}
		}

		bool Initialize(SDL_Window* gameWindow)
		{
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
}
