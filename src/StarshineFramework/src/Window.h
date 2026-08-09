#pragma once
#include "Common/Types.h"
#include <SDL2/SDL_video.h>
#include <vector>

namespace Starshine
{
	struct DisplayMode
	{
		ivec2 Resolution{};
		i32 RefreshRate{};
		SDL_DisplayMode SDLDisplayMode{};
	};

	inline bool operator==(const DisplayMode& a, const DisplayMode& b)
	{
		return SDL_memcmp(&a, &b, sizeof(DisplayMode)) == 0;
	}

	class Window : public NonCopyable
	{
	public:
		Window(std::string_view title, i32 width, i32 height, SDL_WindowFlags flags);
		~Window();

	public:
		bool Exists() const;

		void SetSize(const ivec2& newSize);
		ivec2 GetSize() const;
		void CenterWindow();

		void SetResizing(bool allow);
		bool CanBeResized() const;

		void SetFullscreen(bool fullscreen);
		bool GetFullscreen() const;

		void SetTitle(std::string_view title);
		std::string_view GetTitle() const;

		SDL_Window* GetBaseWindow();

	public:
		const std::vector<DisplayMode>& GetDisplayModes() const;
		DisplayMode GetCurrentDisplayMode() const;
		const DisplayMode* GetNativeDisplayMode() const;
		void SetDisplayMode(const DisplayMode& mode);

	private:
		SDL_Window* baseWindow{};
		std::string windowTitle{};
	};
}
