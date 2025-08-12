#pragma once
#include "common/types.h"
#include "common/color.h"
#include <SDL2/SDL.h>

namespace Starshine::GFX
{
	enum class RendererBackend : u32
	{
		OpenGL,
		D3D9, // TODO: Implement

		Count
	};

	constexpr const char* RendererBackendNames[DIVA::EnumCount<RendererBackend>()] =
	{
		"OpenGL",
		"D3D9"
	};

	enum ClearFlags : u8
	{
		ClearFlags_Color = 1 << 0,
		ClearFlags_Depth = 1 << 1,
		ClearFlags_Stencil = 1 << 2
	};

	class Renderer : NonCopyable
	{
	public:
		Renderer(RendererBackend backend);
		~Renderer() = default;

	public:
		static void CreateInstance(RendererBackend backend);
		static void DeleteInstance();

		static Renderer& GetInstance();

	public:
		bool Initialize(SDL_Window* gameWindow);

		void Clear(ClearFlags flags, Common::Color color, f32 depth, u8 stencil);
		void SwapBuffers();

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
