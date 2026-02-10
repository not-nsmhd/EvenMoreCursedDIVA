#pragma once
#include "Common/Types.h"
#include <SDL2/SDL_video.h>

namespace Starshine
{
	class Window : public NonCopyable
	{
	public:
		Window(std::string_view title, i32 width, i32 height, SDL_WindowFlags flags);
		~Window();

	public:
		bool Exists() const;

		void SetSize(const ivec2& newSize);
		ivec2 GetSize() const;

		void SetResizing(bool allow);
		bool CanBeResized() const;

		void SetTitle(std::string_view title);
		std::string_view GetTitle() const;

		SDL_Window* GetBaseWindow();

	private:
		SDL_Window* baseWindow{};
		std::string windowTitle{};
	};
}
