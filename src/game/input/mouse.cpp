#include "mouse.h"

namespace Input
{
	static const u8 MOUSEBUTTONSTATE_LEFT = 0x01;
	static const u8 MOUSEBUTTONSTATE_RIGHT = 0x02;
	static const u8 MOUSEBUTTONSTATE_MIDDLE = 0x04;

	Mouse::Mouse()
	{
	}

	Mouse *Mouse::instance = nullptr;

	Mouse *Mouse::GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new Mouse();
		}

		return instance;
	}

	void Mouse::Poll(SDL_Event& event)
	{
		switch (event.type)
		{
		case SDL_MOUSEMOTION:
			curPosition.x = event.motion.x;
			curPosition.y = event.motion.y;
			relPosition.x = event.motion.xrel;
			relPosition.y = event.motion.yrel;
			break;
		case SDL_MOUSEBUTTONUP:
			{
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT:
					curMouseButtonState &= ~MOUSEBUTTONSTATE_LEFT;
					break;
				case SDL_BUTTON_RIGHT:
					curMouseButtonState &= ~MOUSEBUTTONSTATE_RIGHT;
					break;
				case SDL_BUTTON_MIDDLE:
					curMouseButtonState &= ~MOUSEBUTTONSTATE_MIDDLE;
					break;
				}
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			{
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT:
					curMouseButtonState |= MOUSEBUTTONSTATE_LEFT;
					break;
				case SDL_BUTTON_RIGHT:
					curMouseButtonState |= MOUSEBUTTONSTATE_RIGHT;
					break;
				case SDL_BUTTON_MIDDLE:
					curMouseButtonState |= MOUSEBUTTONSTATE_MIDDLE;
					break;
				}
			}
			break;
		case SDL_MOUSEWHEEL:
			scrollValue.x = event.wheel.x;
			scrollValue.y = -event.wheel.y;
			break;
		}
	}
	
	void Mouse::NextFrame()
	{
		prevPosition = curPosition;
		relPosition = glm::ivec2(0);

		prevMouseButtonState = curMouseButtonState;

		scrollValue = glm::ivec2(0);
	}
	
	glm::ivec2 Mouse::GetCurrentPosition()
	{
		return curPosition;
	}
	
	glm::ivec2 Mouse::GetPreviousPosition()
	{
		return prevPosition;
	}
	
	glm::ivec2 Mouse::GetRelativePosition()
	{
		return relPosition;
	}
	
	bool Mouse::IsMouseButtonDown(MouseButton button)
	{
		switch (button)
		{
			case MouseButton::MOUSE_LEFT:
				return (curMouseButtonState & MOUSEBUTTONSTATE_LEFT) == MOUSEBUTTONSTATE_LEFT;
			case MouseButton::MOUSE_RIGHT:
				return (curMouseButtonState & MOUSEBUTTONSTATE_RIGHT) == MOUSEBUTTONSTATE_RIGHT;
			case MouseButton::MOUSE_MIDDLE:
				return (curMouseButtonState & MOUSEBUTTONSTATE_MIDDLE) == MOUSEBUTTONSTATE_MIDDLE;
		}

		return false;
	}
	
	bool Mouse::IsMouseButtonUp(MouseButton button)
	{
		switch (button)
		{
			case MouseButton::MOUSE_LEFT:
				return (curMouseButtonState & MOUSEBUTTONSTATE_LEFT) == 0;
			case MouseButton::MOUSE_RIGHT:
				return (curMouseButtonState & MOUSEBUTTONSTATE_RIGHT) == 0;
			case MouseButton::MOUSE_MIDDLE:
				return (curMouseButtonState & MOUSEBUTTONSTATE_MIDDLE) == 0;
		}

		return false;
	}
	
	bool Mouse::IsMouseButtonTapped(MouseButton button)
	{
		switch (button)
		{
			case MouseButton::MOUSE_LEFT:
				return (curMouseButtonState & MOUSEBUTTONSTATE_LEFT) == MOUSEBUTTONSTATE_LEFT && (prevMouseButtonState & MOUSEBUTTONSTATE_LEFT) == 0;
			case MouseButton::MOUSE_RIGHT:
				return (curMouseButtonState & MOUSEBUTTONSTATE_RIGHT) == MOUSEBUTTONSTATE_RIGHT && (prevMouseButtonState & MOUSEBUTTONSTATE_RIGHT) == 0;
			case MouseButton::MOUSE_MIDDLE:
				return (curMouseButtonState & MOUSEBUTTONSTATE_MIDDLE) == MOUSEBUTTONSTATE_MIDDLE && (prevMouseButtonState & MOUSEBUTTONSTATE_MIDDLE) == 0;
		}

		return false;
	}
	
	bool Mouse::IsMouseButtonReleased(MouseButton button)
	{
		switch (button)
		{
			case MouseButton::MOUSE_LEFT:
				return (curMouseButtonState & MOUSEBUTTONSTATE_LEFT) == 0 && (prevMouseButtonState & MOUSEBUTTONSTATE_LEFT) == MOUSEBUTTONSTATE_LEFT;
			case MouseButton::MOUSE_RIGHT:
				return (curMouseButtonState & MOUSEBUTTONSTATE_RIGHT) == 0 && (prevMouseButtonState & MOUSEBUTTONSTATE_RIGHT) == MOUSEBUTTONSTATE_RIGHT;
			case MouseButton::MOUSE_MIDDLE:
				return (curMouseButtonState & MOUSEBUTTONSTATE_MIDDLE) == 0 && (prevMouseButtonState & MOUSEBUTTONSTATE_MIDDLE) == MOUSEBUTTONSTATE_MIDDLE;
		}

		return false;
	}
	
	glm::ivec2 Mouse::GetScrollWheelValue()
	{
		return scrollValue;
	}
}
