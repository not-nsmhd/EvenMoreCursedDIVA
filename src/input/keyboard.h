#pragma once
#include <SDL2/SDL.h>
#include "common/int_types.h"
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
}