#pragma once
#include <SDL2/SDL.h>
#include "common/types.h"

namespace Starshine::Input
{
	constexpr SDL_Keycode UnboundKey = SDLK_UNKNOWN;

	struct KeyBind
	{
		SDL_Keycode Primary{ UnboundKey };
		SDL_Keycode Secondary{ UnboundKey };
	};

	class Keyboard : public NonCopyable
	{
	public:
		Keyboard();

	public:
		static void Initialize();
		static void Destroy();

		static void Poll(const SDL_KeyboardEvent& event);
		static void NextFrame();

	public:
		static bool IsKeyDown(SDL_Keycode key);
		static bool IsKeyUp(SDL_Keycode key);
		static bool IsKeyTapped(SDL_Keycode key);
		static bool IsKeyReleased(SDL_Keycode key);

		static bool IsAnyDown(const KeyBind& keybind, bool* primary, bool* secondary);
		static bool IsAnyTapped(const KeyBind& keybind, bool* primary, bool* secondary);
		static bool IsAnyReleased(const KeyBind& keybind, bool* primary, bool* secondary);
	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
