#include "Window.h"
#include "Rendering/Device.h"

namespace Starshine
{
	static std::vector<SDL_DisplayMode> supportedDisplayModes;
	SDL_DisplayMode nativeDisplayMode;

	void InitDisplayModeList()
	{
		i32 dispModeNum = SDL_GetNumDisplayModes(0);
		for (i32 i = dispModeNum - 1; i >= 0; i--)
		{
			SDL_DisplayMode dispMode{};

			SDL_GetDisplayMode(0, i, &dispMode);

			if ((dispMode.w / 16) != (dispMode.h / 9))
				continue;

			supportedDisplayModes.push_back(dispMode);
		}

		SDL_GetDesktopDisplayMode(0, &nativeDisplayMode);
	}

	Window::Window(std::string_view title, i32 width, i32 height, SDL_WindowFlags flags)
		: windowTitle(title)
	{
		baseWindow = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
		InitDisplayModeList();
	}

	Window::~Window()
	{
		SDL_DestroyWindow(baseWindow);
	}

	bool Window::Exists() const
	{
		return baseWindow != nullptr;
	}

	void Window::SetSize(const ivec2& newSize)
	{
		if (newSize.x <= 0 || newSize.y <= 0) { return; }
		SDL_SetWindowSize(baseWindow, newSize.x, newSize.y);
		SDL_SetWindowPosition(baseWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		Rendering::GetDevice()->OnWindowResize(newSize.x, newSize.y);
	}

	ivec2 Window::GetSize() const
	{
		ivec2 result{};
		SDL_GetWindowSizeInPixels(baseWindow, &result.x, &result.y);
		return result;
	}

	void Window::SetPosition(const ivec2& position)
	{
		if (position == CenteredWindowPos)
			SDL_SetWindowPosition(baseWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		else
			SDL_SetWindowPosition(baseWindow, position.x, position.y);
	}

	ivec2 Window::GetPosition() const
	{
		ivec2 result{};
		SDL_GetWindowPosition(baseWindow, &result.x, &result.y);
		return result;
	}

	void Window::CenterWindow()
	{
		SDL_SetWindowPosition(baseWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	void Window::SetResizing(bool allow)
	{
		SDL_SetWindowResizable(baseWindow, allow ? SDL_TRUE : SDL_FALSE);
	}

	bool Window::CanBeResized() const
	{
		Uint32 windowFlags = SDL_GetWindowFlags(baseWindow);
		return (windowFlags & SDL_WINDOW_RESIZABLE) != 0;
	}

	void Window::SetMode(WindowMode mode)
	{
		switch (mode)
		{
		case WindowMode::Window:
			SDL_SetWindowFullscreen(baseWindow, 0);
			break;
		case WindowMode::Fullscreen:
			SDL_SetWindowFullscreen(baseWindow, SDL_WINDOW_FULLSCREEN);
			break;
		case WindowMode::FullscreenWindow:
			SDL_SetWindowFullscreen(baseWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
			break;
		}
	}

	WindowMode Window::GetMode() const
	{
		u32 flags = SDL_GetWindowFlags(baseWindow);
		if (flags & SDL_WINDOW_FULLSCREEN) return WindowMode::Fullscreen;
		else if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) return WindowMode::FullscreenWindow;
		return WindowMode::Window;
	}

	void Window::SetTitle(std::string_view title)
	{
		SDL_SetWindowTitle(baseWindow, title.data());
		windowTitle = title;
	}

	std::string_view Window::GetTitle() const
	{
		return windowTitle;
	}

	SDL_Window* Window::GetBaseWindow()
	{
		return baseWindow;
	}

	const std::vector<SDL_DisplayMode>& Window::GetDisplayModes() const
	{
		return supportedDisplayModes;
	}

	SDL_DisplayMode Window::GetCurrentDisplayMode() const
	{
		SDL_DisplayMode result{};
		SDL_GetCurrentDisplayMode(0, &result);
		return result;
	}

	const SDL_DisplayMode& Window::GetNativeDisplayMode() const
	{
		return nativeDisplayMode;
	}

	void Window::SetDisplayMode(const SDL_DisplayMode& mode)
	{
		SetSize(vec2(mode.w, mode.h));
	}
}
