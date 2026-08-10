#pragma once
#include "Common/Types.h"
#include <SDL2/SDL_video.h>
#include <array>
#include <vector>

namespace Starshine
{
	enum class WindowMode : i8
	{
		Window,
		Fullscreen,
		FullscreenWindow,

		Count
	};

	constexpr std::array<const char*, EnumCount<WindowMode>()> WindowModeNames =
	{
		"Window",
		"Fullscreen",
		"FullscreenWindow"
	};

	constexpr ivec2 CenteredWindowPos = ivec2(-1);

	class Window : public NonCopyable
	{
	public:
		Window(std::string_view title, i32 width, i32 height, SDL_WindowFlags flags);
		~Window();

	public:
		bool Exists() const;

		void SetSize(const ivec2& newSize);
		ivec2 GetSize() const;

		void SetPosition(const ivec2& position);
		ivec2 GetPosition() const;
		void CenterWindow();

		void SetResizing(bool allow);
		bool CanBeResized() const;

		void SetMode(WindowMode mode);
		WindowMode GetMode() const;

		void SetTitle(std::string_view title);
		std::string_view GetTitle() const;

		SDL_Window* GetBaseWindow();

	public:
		const std::vector<SDL_DisplayMode>& GetDisplayModes() const;
		SDL_DisplayMode GetCurrentDisplayMode() const;
		const SDL_DisplayMode& GetNativeDisplayMode() const;
		void SetDisplayMode(const SDL_DisplayMode& mode);

	private:
		SDL_Window* baseWindow{};
		std::string windowTitle{};
	};
}
