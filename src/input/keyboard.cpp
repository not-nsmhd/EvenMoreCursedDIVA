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
		if (scancode == SDL_SCANCODE_UNKNOWN)
		{
			return false;
		}
		return curKeyboardState[scancode] == 1;
	}

	bool Keyboard::IsKeyUp(SDL_Scancode scancode)
	{
		if (scancode == SDL_SCANCODE_UNKNOWN)
		{
			return false;
		}
		return curKeyboardState[scancode] == 0;
	}

	bool Keyboard::IsKeyTapped(SDL_Scancode scancode)
	{
		if (scancode == SDL_SCANCODE_UNKNOWN)
		{
			return false;
		}
		return curKeyboardState[scancode] == 1 && prevKeyboardState[scancode] == 0;
	}

	bool Keyboard::IsKeyReleased(SDL_Scancode scancode)
	{
		if (scancode == SDL_SCANCODE_UNKNOWN)
		{
			return false;
		}
		return curKeyboardState[scancode] == 0 && prevKeyboardState[scancode] == 1;
	}

	KeyBind::KeyBind(Keyboard* keyboard, SDL_Scancode primary, SDL_Scancode alternative)
	{
		this->keyboard = keyboard;
		PrimaryKey = primary;
		AlternativeKey = alternative;
	}

	bool KeyBind::IsDown(bool* primary, bool* alternative) const
	{
		bool prim = keyboard->IsKeyDown(PrimaryKey);
		bool alt = keyboard->IsKeyDown(AlternativeKey);
		if (primary != nullptr)
		{
			*primary = prim;
		}
		if (alternative != nullptr)
		{
			*alternative = alt;
		}
		return prim || alt;
	}

	bool KeyBind::IsUp(bool* primary, bool* alternative) const
	{
		bool prim = keyboard->IsKeyUp(PrimaryKey);
		bool alt = keyboard->IsKeyUp(AlternativeKey);
		if (primary != nullptr)
		{
			*primary = prim;
		}
		if (alternative != nullptr)
		{
			*alternative = alt;
		}
		return prim && alt;
	}

	bool KeyBind::IsTapped(bool* primary, bool* alternative) const
	{
		bool prim = keyboard->IsKeyTapped(PrimaryKey);
		bool alt = keyboard->IsKeyTapped(AlternativeKey);
		if (primary != nullptr)
		{
			*primary = prim;
		}
		if (alternative != nullptr)
		{
			*alternative = alt;
		}
		return prim || alt;
	}

	bool KeyBind::IsReleased(bool* primary, bool* alternative) const
	{
		bool prim = keyboard->IsKeyReleased(PrimaryKey);
		bool alt = keyboard->IsKeyReleased(AlternativeKey);
		if (primary != nullptr)
		{
			*primary = prim;
		}
		if (alternative != nullptr)
		{
			*alternative = alt;
		}
		return prim || alt;
	}
}
