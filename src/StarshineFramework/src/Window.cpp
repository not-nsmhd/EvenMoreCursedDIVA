#include "Window.h"

namespace Starshine
{
	static std::vector<DisplayMode> supportedDisplayModes;
	DisplayMode* currentDisplayMode{};
	DisplayMode* nativeDisplayMode{};

	void InitDisplayModeList()
	{
		i32 dispModeNum = SDL_GetNumDisplayModes(0);
		for (i32 i = dispModeNum - 1; i >= 0; i--)
		{
			SDL_DisplayMode sdlDispMode{};

			SDL_GetDisplayMode(0, i, &sdlDispMode);

			if ((sdlDispMode.w / 16) != (sdlDispMode.h / 9))
				continue;

			DisplayMode& dispMode = supportedDisplayModes.emplace_back();

			dispMode.Resolution = { sdlDispMode.w, sdlDispMode.h };
			dispMode.RefreshRate = sdlDispMode.refresh_rate;
			dispMode.SDLDisplayMode = sdlDispMode;
		}

		nativeDisplayMode = &supportedDisplayModes.back();
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
	}

	ivec2 Window::GetSize() const
	{
		ivec2 result{};
		SDL_GetWindowSizeInPixels(baseWindow, &result.x, &result.y);
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

	void Window::SetFullscreen(bool fullscreen)
	{
		SDL_SetWindowFullscreen(baseWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}

	bool Window::GetFullscreen() const
	{
		return SDL_GetWindowFlags(baseWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP;
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

	const std::vector<DisplayMode>& Window::GetDisplayModes() const
	{
		return supportedDisplayModes;
	}

	DisplayMode Window::GetCurrentDisplayMode() const
	{
		return DisplayMode();
	}

	const DisplayMode* Window::GetNativeDisplayMode() const
	{
		return nativeDisplayMode;
	}

	void Window::SetDisplayMode(const DisplayMode& mode)
	{
		if (SDL_GetWindowFlags(baseWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP)
		{
			SDL_SetWindowDisplayMode(baseWindow, &mode.SDLDisplayMode);
		}
		else
		{
			SDL_SetWindowSize(baseWindow, mode.Resolution.x, mode.Resolution.y);
			SDL_SetWindowPosition(baseWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		}
	}
}
