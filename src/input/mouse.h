#pragma once
#include <SDL2/SDL.h>
#include "common/int_types.h"
#include <glm/vec2.hpp>

namespace Input
{
	enum class MouseButton
	{
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_MIDDLE
	};

	class Mouse
	{
	protected:
		Mouse();
		static Mouse* instance;

	public:
		Mouse(Mouse& other) = delete;
		void operator=(const Mouse&) = delete;

		static Mouse* GetInstance();

		void Poll(SDL_Event& event);
		void NextFrame();

		glm::ivec2 GetCurrentPosition();
		glm::ivec2 GetPreviousPosition();
		glm::ivec2 GetRelativePosition();

		bool IsMouseButtonDown(MouseButton button);
		bool IsMouseButtonUp(MouseButton button);
		bool IsMouseButtonTapped(MouseButton button);
		bool IsMouseButtonReleased(MouseButton button);

		/* Positive Y is down; Negative Y is up */
		glm::ivec2 GetScrollWheelValue();
	private:
		glm::ivec2 curPosition = {};
		glm::ivec2 prevPosition = {};
		glm::ivec2 relPosition = {};

		u8 curMouseButtonState = 0;
		u8 prevMouseButtonState = 0;

		glm::ivec2 scrollValue = {};
	};
}