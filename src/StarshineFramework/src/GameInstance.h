#pragma once
#include "Common/Types.h"
#include "Window.h"
#include <memory>

namespace Starshine
{
	class GameInstance;

	class GameState : public NonCopyable
	{
	public:
		GameState() = default;
		~GameState() = default;
	
	public:
		virtual bool Initialize() = 0;
		virtual bool LoadContent() = 0;

		virtual void UnloadContent() = 0;
		virtual void Destroy() = 0;

		virtual void Update(f64 deltaTime_milliseconds) = 0;
		virtual void Draw(f64 deltaTime_milliseconds) = 0;

		virtual std::string_view GetStateName() const = 0;

	public:
		GameInstance* GameInstance{};
	};

	class GameInstance
	{
	public:
		GameInstance();
		~GameInstance();

	public:
		Window* const GetWindow();

	public:
		bool Initialize();
		void EnterLoop();

	public:
		bool SetState(std::unique_ptr<GameState> state);

	private:
		std::unique_ptr<Window> GameWindow{ nullptr };

		struct Impl;
		std::unique_ptr<Impl> impl{ nullptr };
	};
}
