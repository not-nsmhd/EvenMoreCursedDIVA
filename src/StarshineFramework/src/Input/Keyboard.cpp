#include "Keyboard.h"
#include <array>
#include "Common/Logging/Logging.h"

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
			if (key == UnboundKey)
			{
				return false;
			}

			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			return State.CurrentKeyState[scancode] == true;
		}

		bool IsKeyUp(SDL_Keycode key)
		{
			if (key == UnboundKey)
			{
				return false;
			}

			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			return State.CurrentKeyState[scancode] == false;
		}

		bool IsKeyTapped(SDL_Keycode key)
		{
			if (key == UnboundKey)
			{
				return false;
			}

			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			return (State.CurrentKeyState[scancode] == true) && (State.PreviousKeyState[scancode] == false);
		}

		bool IsKeyReleased(SDL_Keycode key)
		{
			if (key == UnboundKey)
			{
				return false;
			}

			SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
			return (State.CurrentKeyState[scancode] == false) && (State.PreviousKeyState[scancode] == true);
		}
	};

	std::unique_ptr<Keyboard> GlobalInstance{};

	Keyboard::Keyboard() : impl(std::make_unique<Impl>())
	{
	}

	void Keyboard::Initialize()
	{
		GlobalInstance = std::make_unique<Keyboard>();
	}

	void Keyboard::Destroy()
	{
		GlobalInstance = nullptr;
	}

	void Keyboard::Poll(const SDL_KeyboardEvent& event)
	{
		GlobalInstance->impl->Poll(event);
	}

	void Keyboard::NextFrame()
	{
		GlobalInstance->impl->NextFrame();
	}

	bool Keyboard::IsKeyDown(const SDL_Keycode& key)
	{
		return GlobalInstance->impl->IsKeyDown(key);
	}

	bool Keyboard::IsKeyUp(const SDL_Keycode& key)
	{
		return GlobalInstance->impl->IsKeyUp(key);
	}

	bool Keyboard::IsKeyTapped(const SDL_Keycode& key)
	{
		return GlobalInstance->impl->IsKeyTapped(key);
	}

	bool Keyboard::IsKeyReleased(const SDL_Keycode& key)
	{
		return GlobalInstance->impl->IsKeyReleased(key);
	}

	bool Keyboard::IsAnyDown(const KeyBind& keybind, bool* primary, bool* secondary)
	{
		bool primKey = IsKeyDown(keybind.Primary);
		bool secondKey = IsKeyDown(keybind.Secondary);

		if (primary != nullptr)
		{
			*primary = primKey;
		}

		if (secondary != nullptr)
		{
			*secondary = secondKey;
		}

		return primKey || secondKey;
	}

	bool Keyboard::IsAnyTapped(const KeyBind& keybind, bool* primary, bool* secondary)
	{
		bool primKey = IsKeyTapped(keybind.Primary);
		bool secondKey = IsKeyTapped(keybind.Secondary);

		if (primary != nullptr)
		{
			*primary = primKey;
		}

		if (secondary != nullptr)
		{
			*secondary = secondKey;
		}

		return primKey || secondKey;
	}

	bool Keyboard::IsAnyReleased(const KeyBind& keybind, bool* primary, bool* secondary)
	{
		bool primKey = IsKeyReleased(keybind.Primary);
		bool secondKey = IsKeyReleased(keybind.Secondary);

		if (primary != nullptr)
		{
			*primary = primKey;
		}

		if (secondary != nullptr)
		{
			*secondary = secondKey;
		}

		return primKey || secondKey;
	}
}
