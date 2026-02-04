#pragma once
#include "Common/Types.h"
#include <SDL2/SDL.h>
#include <memory>

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
		static bool IsKeyDown(const SDL_Keycode& key);
		static bool IsKeyUp(const SDL_Keycode& key);
		static bool IsKeyTapped(const SDL_Keycode& key);
		static bool IsKeyReleased(const SDL_Keycode& key);

		static bool IsAnyDown(const KeyBind& keybind, bool* primary, bool* secondary);
		static bool IsAnyTapped(const KeyBind& keybind, bool* primary, bool* secondary);
		static bool IsAnyReleased(const KeyBind& keybind, bool* primary, bool* secondary);
	private:
		struct Impl;
		std::unique_ptr<Impl> impl = nullptr;
	};
}
