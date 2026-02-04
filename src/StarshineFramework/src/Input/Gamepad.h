#pragma once
#include "Common/Types.h"
#include "GamepadTypes.h"
#include <memory>

namespace Starshine::Input
{
	struct GamepadBind
	{
		GamepadButton Primary{ GamepadButton::Unknown };
		GamepadButton Alternative{ GamepadButton::Unknown };
	};

	class Gamepad : public NonCopyable
	{
	public:
		Gamepad();

	public:
		static void Initialize();
		static void Destroy();

		static void Poll();
		static void NextFrame();

	public:
		static void Connect(int index);
		static void Disconnect();
		
		static bool IsConnected();

	public:
		static bool IsButtonDown(const GamepadButton& button);
		static bool IsButtonUp(const GamepadButton& button);
		static bool IsButtonTapped(const GamepadButton& button);
		static bool IsButtonReleased(const GamepadButton& button);

	public:
		static bool IsAnyButtonDown(const GamepadBind& bind, bool* primary, bool* alternative);
		static bool IsAnyButtonTapped(const GamepadBind& bind, bool* primary, bool* alternative);
		static bool IsAnyButtonReleased(const GamepadBind& bind, bool* primary, bool* alternative);

	public:
		static f32 GetAxis(const GamepadAxis& axis);
		static f32 GetAxisOnPreviousFrame(const GamepadAxis& axis);

		static bool IsStickHeld(const GamepadStick& stick);
		static bool IsStickCentered(const GamepadStick& stick);
		static bool IsStickPulled(const GamepadStick& stick);
		static bool IsStickReleased(const GamepadStick& stick);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};
	};
}
