#pragma once
#include <SDL2/SDL.h>
#include "common/types.h"

namespace Starshine::Input
{
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
	private:
		struct Impl;
		Impl* impl = nullptr;
	};

	/*class KeyBind
	{
	public:
		KeyBind(Keyboard* keyboard, SDL_Scancode primary, SDL_Scancode alternative);
		KeyBind(KeyBind& other) = delete;
		void operator=(const KeyBind&) = delete;

		SDL_Scancode PrimaryKey;
		SDL_Scancode AlternativeKey;

		bool IsDown(bool* primary, bool* alternative) const;
		bool IsUp(bool* primary, bool* alternative) const;
		bool IsTapped(bool* primary, bool* alternative) const;
		bool IsReleased(bool* primary, bool* alternative) const;
	public:
		static constexpr SDL_Scancode UnsetScancode = SDL_SCANCODE_UNKNOWN;
	private:
		Keyboard* keyboard = nullptr;
	};*/
}
