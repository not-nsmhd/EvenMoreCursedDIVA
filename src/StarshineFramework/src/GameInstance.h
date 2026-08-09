#pragma once
#include "Common/Types.h"
#include "Window.h"
#include <vector>
#include <memory>
#include <functional>
#include "TimeSpan.h"

namespace Starshine
{
	class GameInstance;

	class GameState : NonCopyable
	{
	public:
		GameState() = default;
		~GameState() = default;
	
	public:
		virtual bool Initialize() = 0;
		virtual bool LoadContent() = 0;

		virtual void UnloadContent() = 0;
		virtual void Destroy() = 0;

		virtual void Update(GameTime& gameTime) = 0;
		virtual void Draw(GameTime& gameTime) = 0;

		virtual std::string_view GetStateName() = 0;

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
		bool Initialize(bool initImGui);
		void EnterLoop();
		void Destroy();

	public:
		bool SetState(GameState* state);
		void Quit();

	private:
		std::unique_ptr<Window> GameWindow{ nullptr };

		struct Impl;
		std::unique_ptr<Impl> impl{ nullptr };
	};
}
