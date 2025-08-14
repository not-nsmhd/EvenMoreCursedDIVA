#pragma once
#include "common/types.h"
#include "gfx/new/Core/IBackend.h"

namespace Starshine::GFX::Core::OpenGL
{
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
		void Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil);
		void SwapBuffers();

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
