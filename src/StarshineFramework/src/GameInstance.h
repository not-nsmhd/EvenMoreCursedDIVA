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

		virtual void Update(GameTime& gameTime) = 0;
		virtual void Draw(GameTime& gameTime) = 0;

		virtual i64 GetStateID() const = 0;

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
		template<typename T>
		void RegisterState();

		GameState* GetStateInstance(i64 stateID);
		bool SetState(i64 stateID);

	private:
		std::unique_ptr<Window> GameWindow{ nullptr };

		struct Impl;
		std::unique_ptr<Impl> impl{ nullptr };

		std::vector<std::unique_ptr<GameState>> GameStates;
	};

	template<typename T>
	inline void Starshine::GameInstance::RegisterState()
	{
		static_assert(std::is_base_of_v<GameState, T>, "T must be inherited from GameState");
		GameStates.push_back(std::make_unique<T>());
	}
}
