#pragma once
#include <SDL2/SDL.h>
#include "../common/types.h"
#include <unordered_map>

namespace Input
{
	class Keyboard
	{
	protected:
		Keyboard();
		static Keyboard* instance;

	public:
		Keyboard(Keyboard& other) = delete;
		void operator=(const Keyboard&) = delete;

		static Keyboard* GetInstance();

		void Poll(SDL_Event& event);
		void NextFrame();

		bool IsKeyDown(SDL_Scancode scancode);
		bool IsKeyUp(SDL_Scancode scancode);
		bool IsKeyTapped(SDL_Scancode scancode);
		bool IsKeyReleased(SDL_Scancode scancode);
	private:
		std::unordered_map<SDL_Scancode, u8> curKeyboardState = {};
		std::unordered_map<SDL_Scancode, u8> prevKeyboardState = {};
	};

	class KeyBind
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
	};
}
