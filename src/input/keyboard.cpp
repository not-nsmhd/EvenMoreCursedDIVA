#include "keyboard.h"

namespace Input
{
	Keyboard::Keyboard()
	{
		curKeyboardState = std::unordered_map<SDL_Scancode, u8>(SDL_NUM_SCANCODES);
		prevKeyboardState = std::unordered_map<SDL_Scancode, u8>(SDL_NUM_SCANCODES);
	}

	Keyboard *Keyboard::instance = nullptr;

	Keyboard *Keyboard::GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new Keyboard();
		}

		return instance;
	}

	void Keyboard::Poll(SDL_Event& event)
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			curKeyboardState[event.key.keysym.scancode] = 1;
			break;
		case SDL_KEYUP:
			curKeyboardState[event.key.keysym.scancode] = 0;
			break;
		}
	}
	
	void Keyboard::NextFrame()
	{
		prevKeyboardState = curKeyboardState;
	}

	bool Keyboard::IsKeyDown(SDL_Scancode scancode)
	{
		return curKeyboardState[scancode] == 1;
	}

	bool Keyboard::IsKeyUp(SDL_Scancode scancode)
	{
		return curKeyboardState[scancode] == 0;
	}

	bool Keyboard::IsKeyTapped(SDL_Scancode scancode)
	{
		return curKeyboardState[scancode] == 1 && prevKeyboardState[scancode] == 0;
	}

	bool Keyboard::IsKeyReleased(SDL_Scancode scancode)
	{
		return curKeyboardState[scancode] == 0 && prevKeyboardState[scancode] == 1;
	}
}
