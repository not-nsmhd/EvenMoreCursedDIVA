#include "Keyboard.h"
#include <array>

namespace Starshine::Input
{
	using std::array;

	struct Keyboard::Impl
	{
		struct StateData
		{
			array<bool, SDL_NUM_SCANCODES> CurrentKeyState;
			array<bool, SDL_NUM_SCANCODES> PreviousKeyState;
		} State;

		void Poll(const SDL_KeyboardEvent& event)
		{
			size_t keyStateIndex = static_cast<size_t>(event.keysym.scancode);
			State.CurrentKeyState[keyStateIndex] = (event.state == SDL_PRESSED);
		}

		void NextFrame()
		{
			State.PreviousKeyState = State.CurrentKeyState;
		}

		bool IsKeyDown(SDL_Keycode key)
		{
			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			return State.CurrentKeyState[scancode] == true;
		}

		bool IsKeyUp(SDL_Keycode key)
		{
			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			return State.CurrentKeyState[scancode] == false;
		}

		bool IsKeyTapped(SDL_Keycode key)
		{
			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			return (State.CurrentKeyState[scancode] == true) && (State.PreviousKeyState[scancode] == false);
		}

		bool IsKeyReleased(SDL_Keycode key)
		{
			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			return (State.CurrentKeyState[scancode] == false) && (State.PreviousKeyState[scancode] == true);
		}
	};

	Keyboard* GlobalInstance = nullptr;

	Keyboard::Keyboard() : impl(new Impl())
	{
	}

	void Keyboard::Initialize()
	{
		GlobalInstance = new Keyboard();
	}

	void Keyboard::Destroy()
	{
		delete GlobalInstance;
	}

	void Keyboard::Poll(const SDL_KeyboardEvent& event)
	{
		GlobalInstance->impl->Poll(event);
	}

	void Keyboard::NextFrame()
	{
		GlobalInstance->impl->NextFrame();
	}

	bool Keyboard::IsKeyDown(SDL_Keycode key)
	{
		return GlobalInstance->impl->IsKeyDown(key);
	}

	bool Keyboard::IsKeyUp(SDL_Keycode key)
	{
		return GlobalInstance->impl->IsKeyUp(key);
	}

	bool Keyboard::IsKeyTapped(SDL_Keycode key)
	{
		return GlobalInstance->impl->IsKeyTapped(key);
	}

	bool Keyboard::IsKeyReleased(SDL_Keycode key)
	{
		return GlobalInstance->impl->IsKeyReleased(key);
	}
}
