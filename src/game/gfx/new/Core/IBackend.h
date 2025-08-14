#pragma once
#include "common/types.h"
#include "common/color.h"
#include "gfx/new/Types.h"
#include <SDL2/SDL.h>

namespace Starshine::GFX::Core
{
	class IBackend
	{
	public:
		virtual ~IBackend() = default;

	public:
		virtual bool Initialize(SDL_Window* gameWindow) = 0;
		virtual void Destroy() = 0;

	public:
		virtual RendererBackendType GetType() const = 0;

	public:
		virtual void Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil) = 0;
		virtual void SwapBuffers() = 0;
	};
}
