#include <glad/glad.h>
#include "Renderer.h"
#include "util/logging.h"

namespace Starshine::GFX
{
	using namespace Logging;
	constexpr const char* LogName = "Starshine::GFX";

	Renderer* RendererInstance = nullptr;

	struct Renderer::Impl
	{
		SDL_Window* GameWindow{};
		SDL_GLContext GLContext{};

		bool Initialize(SDL_Window* gameWindow)
		{
			u32 windowFlags = SDL_GetWindowFlags(gameWindow);
			if ((windowFlags & SDL_WINDOW_OPENGL) != SDL_WINDOW_OPENGL)
			{
				LogError(LogName, "Game window is not an OpenGL window");
				return false;
			}

			GameWindow = gameWindow;
			if ((GLContext = SDL_GL_CreateContext(GameWindow)) == NULL)
			{
				char message[512] = {};
				SDL_snprintf(message, 511, "Failed to create an OpenGL context.\nError: %s", SDL_GetError());

				LogError(LogName, message);
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (GFX)", message, GameWindow);

				return false;
			}

			if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
			{
				LogError(LogName, "(GLAD) Failed to load OpenGL functions");
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (GLAD)", "Failed to load OpenGL functions", GameWindow);
				SDL_GL_DeleteContext(GLContext);
				return false;
			}

			//SDL_GL_SetSwapInterval(1);

			LogInfo(LogName, "OpenGL Version: %s", glGetString(GL_VERSION));
			LogInfo(LogName, "OpenGL Renderer: %s", glGetString(GL_RENDERER));

			//glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			glEnable(GL_VERTEX_PROGRAM_ARB);
			glEnable(GL_FRAGMENT_PROGRAM_ARB);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			return true;
		}

		void Clear(ClearFlags flags, Common::Color color, f32 depth, u8 stencil)
		{
			GLenum clearFlags = 0;

			if ((flags & ClearFlags::ClearFlags_Color) != 0)
			{
				glClearColor(
					static_cast<float>(color.R) / 255.0f,
					static_cast<float>(color.G) / 255.0f,
					static_cast<float>(color.B) / 255.0f,
					static_cast<float>(color.A) / 255.0f);

				clearFlags |= GL_COLOR_BUFFER_BIT;
			}

			if ((flags & ClearFlags::ClearFlags_Depth) != 0)
			{
				glClearDepth(static_cast<double>(depth));
				clearFlags |= GL_DEPTH_BUFFER_BIT;
			}

			if ((flags & ClearFlags::ClearFlags_Stencil) != 0)
			{
				glClearStencil(stencil);
				clearFlags |= GL_STENCIL_BUFFER_BIT;
			}

			glClear(clearFlags);
		}
		
		void SwapBuffers()
		{
			SDL_GL_SwapWindow(GameWindow);
		}
	};

	Renderer::Renderer(RendererBackend backend) : impl(new Impl())
	{
	}

	void Renderer::CreateInstance(RendererBackend backend)
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

	Renderer& Renderer::GetInstance()
	{
		assert(RendererInstance != nullptr);
		return *RendererInstance;
	}

	bool Renderer::Initialize(SDL_Window* gameWindow)
	{
		return impl->Initialize(gameWindow);
	}

	void Renderer::Clear(ClearFlags flags, Common::Color color, f32 depth, u8 stencil)
	{
		impl->Clear(flags, color, depth, stencil);
	}

	void Renderer::SwapBuffers()
	{
		impl->SwapBuffers();
	}
}
