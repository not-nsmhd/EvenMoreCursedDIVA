#include "Window.h"

namespace Starshine
{
	Window::Window(std::string_view title, i32 width, i32 height, SDL_WindowFlags flags)
		: windowTitle(title)
	{
		baseWindow = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
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

	void Window::SetResizing(bool allow)
	{
		SDL_SetWindowResizable(baseWindow, allow ? SDL_TRUE : SDL_FALSE);
	}

	bool Window::CanBeResized() const
	{
		Uint32 windowFlags = SDL_GetWindowFlags(baseWindow);
		return (windowFlags & SDL_WINDOW_RESIZABLE) != 0;
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
}
