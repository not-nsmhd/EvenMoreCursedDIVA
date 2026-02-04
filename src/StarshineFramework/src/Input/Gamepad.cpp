#include "Gamepad.h"
#include <SDL2/SDL_gamecontroller.h>
#include "Common/Logging/Logging.h"

namespace Starshine::Input
{
	constexpr const char* LogName = "Starshine::Input::Gamepad";

	namespace ConversionTables
	{
		constexpr std::array<SDL_GameControllerButton, EnumCount<GamepadButton>()> SDLGameControllerButtons
		{
			SDL_CONTROLLER_BUTTON_B,
			SDL_CONTROLLER_BUTTON_A,
			SDL_CONTROLLER_BUTTON_X,
			SDL_CONTROLLER_BUTTON_Y,

			SDL_CONTROLLER_BUTTON_START,
			SDL_CONTROLLER_BUTTON_BACK,

			SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
			SDL_CONTROLLER_BUTTON_INVALID, // NOTE: Triggers are evaluated through SDL_GameControllerGetAxis function

			SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
			SDL_CONTROLLER_BUTTON_INVALID,

			SDL_CONTROLLER_BUTTON_DPAD_LEFT,
			SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
			SDL_CONTROLLER_BUTTON_DPAD_UP,
			SDL_CONTROLLER_BUTTON_DPAD_DOWN
		};

		constexpr std::array<SDL_GameControllerAxis, EnumCount<GamepadAxis>()> SDLGameControllerAxis
		{
			SDL_CONTROLLER_AXIS_LEFTX,
			SDL_CONTROLLER_AXIS_LEFTY,

			SDL_CONTROLLER_AXIS_RIGHTX,
			SDL_CONTROLLER_AXIS_RIGHTY,

			SDL_CONTROLLER_AXIS_TRIGGERLEFT,
			SDL_CONTROLLER_AXIS_TRIGGERRIGHT
		};
	}

	struct Gamepad::Impl
	{
		static constexpr i16 TriggerPressThreshold = 16384;
		static constexpr i16 StickHoldThreshold = 4096;

		static constexpr i16 StickPullThreshold = 4096;
		static constexpr i16 StickPullThreshold_Delta = 6144;

		SDL_GameController* SDLController{};
		bool IsConnected{};

		struct StateData
		{
			std::array<bool, SDL_CONTROLLER_BUTTON_MAX> CurrentButtonState;
			std::array<bool, SDL_CONTROLLER_BUTTON_MAX> PreviousButtonState;

			std::array<i16, SDL_CONTROLLER_AXIS_MAX> CurrentAxisState;
			std::array<i16, SDL_CONTROLLER_AXIS_MAX> PreviousAxisState;
		} State{};

		Impl()
		{
		}

		void Poll()
		{
			if (!IsConnected) { return; }

			for (size_t i = 0; i < State.CurrentAxisState.size(); i++)
			{
				State.CurrentAxisState[i] = (SDL_GameControllerGetAxis(SDLController, static_cast<SDL_GameControllerAxis>(i)));
			}

			for (size_t i = 0; i < State.CurrentButtonState.size(); i++)
			{
				State.CurrentButtonState[i] = (SDL_GameControllerGetButton(SDLController, static_cast<SDL_GameControllerButton>(i)) == 1);
			}
		}

		void NextFrame()
		{
			if (!IsConnected) { return; }
			State.PreviousButtonState = State.CurrentButtonState;
			State.PreviousAxisState = State.CurrentAxisState;
		}

		void Connect(int sdlGamepadIndex)
		{
			if (!IsConnected)
			{
				SDLController = SDL_GameControllerOpen(sdlGamepadIndex);
				if (SDLController == NULL)
				{
					LogError(LogName, "Failed to open game controller at index %d. Error: %s", sdlGamepadIndex, SDL_GetError());
					return;
				}
				IsConnected = true;
			}
		}

		void Disconnect()
		{
			if (IsConnected)
			{
				SDL_GameControllerClose(SDLController);
				IsConnected = false;
			}
		}

		bool IsButtonDown(const GamepadButton& button)
		{
			if (!IsConnected || button == GamepadButton::Unknown) { return false; }
			if (button == GamepadButton::L2 || button == GamepadButton::R2)
			{ 
				return GetAxis((button == GamepadButton::L2) ? GamepadAxis::L2 : GamepadAxis::R2, false) >= TriggerPressThreshold;
			}

			SDL_GameControllerButton sdlButton = ConversionTables::SDLGameControllerButtons[static_cast<size_t>(button)];
			return State.CurrentButtonState[sdlButton] == true;
		}

		bool IsButtonUp(const GamepadButton& button)
		{
			if (!IsConnected || button == GamepadButton::Unknown) { return false; }
			if (button == GamepadButton::L2 || button == GamepadButton::R2)
			{
				if (button == GamepadButton::L2 || button == GamepadButton::R2)
				{
					return GetAxis((button == GamepadButton::L2) ? GamepadAxis::L2 : GamepadAxis::R2, false) < TriggerPressThreshold;
				}
			}

			SDL_GameControllerButton sdlButton = ConversionTables::SDLGameControllerButtons[static_cast<size_t>(button)];
			return State.CurrentButtonState[sdlButton] == false;
		}

		bool IsButtonTapped(const GamepadButton& button)
		{
			if (!IsConnected || button == GamepadButton::Unknown) { return false; }
			if (button == GamepadButton::L2 || button == GamepadButton::R2)
			{
				if (button == GamepadButton::L2 || button == GamepadButton::R2)
				{
					GamepadAxis axis = (button == GamepadButton::L2) ? GamepadAxis::L2 : GamepadAxis::R2;
					return (GetAxis(axis, false) >= TriggerPressThreshold) && (GetAxis(axis, true) < TriggerPressThreshold);
				}
			}

			SDL_GameControllerButton sdlButton = ConversionTables::SDLGameControllerButtons[static_cast<size_t>(button)];
			return (State.CurrentButtonState[sdlButton] == true) && (State.PreviousButtonState[sdlButton] == false);
		}

		bool IsButtonReleased(const GamepadButton& button)
		{
			if (!IsConnected || button == GamepadButton::Unknown) { return false; }
			if (button == GamepadButton::L2 || button == GamepadButton::R2)
			{
				if (button == GamepadButton::L2 || button == GamepadButton::R2)
				{
					GamepadAxis axis = (button == GamepadButton::L2) ? GamepadAxis::L2 : GamepadAxis::R2;
					return (GetAxis(axis, false) >= TriggerPressThreshold) && (GetAxis(axis, true) < TriggerPressThreshold);
				}
			}

			SDL_GameControllerButton sdlButton = ConversionTables::SDLGameControllerButtons[static_cast<size_t>(button)];
			return (State.CurrentButtonState[sdlButton] == false) && (State.PreviousButtonState[sdlButton] == true);
		}

		i16 GetAxis(const GamepadAxis& axis, bool previousFrame)
		{
			if (!IsConnected) { return 0; }

			SDL_GameControllerAxis sdlAxis = ConversionTables::SDLGameControllerAxis[static_cast<size_t>(axis)];
			return previousFrame ? State.PreviousAxisState[sdlAxis] : State.CurrentAxisState[sdlAxis];
		}

		f32 GetAxisNormalized(const GamepadAxis& axis, bool previousFrame)
		{
			if (!IsConnected) { return 0.0f; }
			return static_cast<f32>(GetAxis(axis, previousFrame)) / static_cast<f32>(std::numeric_limits<i16>::max());
		}

		void GetStickAxisValues(const GamepadStick& stick, i16& x, i16& y, bool previousFrame)
		{
			auto& state = previousFrame ? State.PreviousAxisState : State.CurrentAxisState;

			switch (stick)
			{
			case GamepadStick::Left:
				x = state[SDL_CONTROLLER_AXIS_LEFTX];
				y = state[SDL_CONTROLLER_AXIS_LEFTY];
				return;
			case GamepadStick::Right:
				x = state[SDL_CONTROLLER_AXIS_RIGHTX];
				y = state[SDL_CONTROLLER_AXIS_RIGHTY];
				return;
			}
		}

		bool IsStickHeld(const GamepadStick& stick)
		{
			if (!IsConnected) { return false; }

			i16 stickX = 0;
			i16 stickY = 0;

			GetStickAxisValues(stick, stickX, stickY, false);
			return (std::abs(stickX) >= StickHoldThreshold) || (std::abs(stickY) >= StickHoldThreshold);
		}

		bool IsStickCentered(const GamepadStick& stick)
		{
			if (!IsConnected) { return false; }

			i16 stickX = 0;
			i16 stickY = 0;

			GetStickAxisValues(stick, stickX, stickY, false);
			return (std::abs(stickX) < StickHoldThreshold) || (std::abs(stickY) < StickHoldThreshold);
		}

		bool IsStickPulled(const GamepadStick& stick)
		{
			if (!IsConnected) { return false; }

			i16 stickX_cur = 0;
			i16 stickY_cur = 0;

			i16 stickX_prev = 0;
			i16 stickY_prev = 0;

			GetStickAxisValues(stick, stickX_cur, stickY_cur, false);
			GetStickAxisValues(stick, stickX_prev, stickY_prev, true);
			return ((std::abs(stickY_cur - stickY_prev) >= StickPullThreshold_Delta) && (std::abs(stickY_prev) < StickPullThreshold)) ||
				((std::abs(stickX_cur - stickX_prev) >= StickPullThreshold_Delta) && (std::abs(stickX_prev) < StickPullThreshold));
		}

		bool IsStickReleased(const GamepadStick& stick)
		{
			if (!IsConnected) { return false; }

			i16 stickX_cur = 0;
			i16 stickY_cur = 0;

			i16 stickX_prev = 0;
			i16 stickY_prev = 0;

			GetStickAxisValues(stick, stickX_cur, stickY_cur, false);
			GetStickAxisValues(stick, stickX_prev, stickY_prev, true);
			return ((std::abs(stickY_cur - stickY_prev) < StickPullThreshold_Delta) && (std::abs(stickY_prev) >= StickPullThreshold)) ||
				((std::abs(stickX_cur - stickX_prev) < StickPullThreshold_Delta) && (std::abs(stickX_prev) >= StickPullThreshold));
		}
	};

	std::unique_ptr<Gamepad> GlobalInstance{};

	Gamepad::Gamepad() : impl(std::make_unique<Impl>())
	{
	}

	void Gamepad::Initialize()
	{
		GlobalInstance = std::make_unique<Gamepad>();
	}

	void Gamepad::Destroy()
	{
		Disconnect();
		GlobalInstance = nullptr;
	}

	void Gamepad::Poll()
	{
		GlobalInstance->impl->Poll();
	}

	void Gamepad::NextFrame()
	{
		GlobalInstance->impl->NextFrame();
	}

	void Gamepad::Connect(int index)
	{
		GlobalInstance->impl->Connect(index);
	}

	void Gamepad::Disconnect()
	{
		GlobalInstance->impl->Disconnect();
	}

	bool Gamepad::IsConnected()
	{
		return GlobalInstance->impl->IsConnected;
	}

	bool Gamepad::IsButtonDown(const GamepadButton& button)
	{
		return GlobalInstance->impl->IsButtonDown(button);
	}

	bool Gamepad::IsButtonUp(const GamepadButton& button)
	{
		return GlobalInstance->impl->IsButtonUp(button);
	}

	bool Gamepad::IsButtonTapped(const GamepadButton& button)
	{
		return GlobalInstance->impl->IsButtonTapped(button);
	}

	bool Gamepad::IsButtonReleased(const GamepadButton& button)
	{
		return GlobalInstance->impl->IsButtonReleased(button);
	}

	bool Gamepad::IsAnyButtonDown(const GamepadBind& bind, bool* primary, bool* alternative)
	{
		bool primButton = GlobalInstance->impl->IsButtonDown(bind.Primary);
		bool altButton = GlobalInstance->impl->IsButtonDown(bind.Alternative);

		if (primary != nullptr) { *primary = primButton; }
		if (alternative != nullptr) { *alternative = altButton; }

		return primButton || altButton;
	}

	bool Gamepad::IsAnyButtonTapped(const GamepadBind& bind, bool* primary, bool* alternative)
	{
		bool primButton = GlobalInstance->impl->IsButtonTapped(bind.Primary);
		bool altButton = GlobalInstance->impl->IsButtonTapped(bind.Alternative);

		if (primary != nullptr) { *primary = primButton; }
		if (alternative != nullptr) { *alternative = altButton; }

		return primButton || altButton;
	}

	bool Gamepad::IsAnyButtonReleased(const GamepadBind& bind, bool* primary, bool* alternative)
	{
		bool primButton = GlobalInstance->impl->IsButtonReleased(bind.Primary);
		bool altButton = GlobalInstance->impl->IsButtonReleased(bind.Alternative);

		if (primary != nullptr) { *primary = primButton; }
		if (alternative != nullptr) { *alternative = altButton; }

		return primButton || altButton;
	}

	f32 Gamepad::GetAxis(const GamepadAxis& axis)
	{
		return GlobalInstance->impl->GetAxisNormalized(axis, false);
	}

	f32 Gamepad::GetAxisOnPreviousFrame(const GamepadAxis& axis)
	{
		return GlobalInstance->impl->GetAxisNormalized(axis, true);
	}

	bool Gamepad::IsStickHeld(const GamepadStick& stick)
	{
		return GlobalInstance->impl->IsStickHeld(stick);
	}

	bool Gamepad::IsStickCentered(const GamepadStick& stick)
	{
		return GlobalInstance->impl->IsStickCentered(stick);
	}

	bool Gamepad::IsStickPulled(const GamepadStick& stick)
	{
		return GlobalInstance->impl->IsStickPulled(stick);
	}

	bool Gamepad::IsStickReleased(const GamepadStick& stick)
	{
		return GlobalInstance->impl->IsStickReleased(stick);
	}
}
