#pragma once
#include "Common/Types.h"
#include <SDL2/SDL.h>
#include <memory>

namespace Starshine::Input
{
	enum class MouseButton : u8
	{
		Left,
		Right,
		Middle,

		Count
	};

	class Mouse : public NonCopyable
	{
	public:
		Mouse();

	public:
		static void Initialize();
		static void Destroy();

		static void Poll(const SDL_Event& event);
		static void NextFrame();

	public:
		static bool IsButtonDown(const MouseButton& button);
		static bool IsButtonUp(const MouseButton& button);
		static bool IsButtonTapped(const MouseButton& button);
		static bool IsButtonReleased(const MouseButton& button);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl = nullptr;
	};
}
