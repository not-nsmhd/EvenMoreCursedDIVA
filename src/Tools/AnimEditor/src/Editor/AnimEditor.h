#pragma once
#include <Common/Types.h>
#include <GameInstance.h>

namespace Starshine
{
	class AnimEditor : public GameState
	{
	public:
		AnimEditor();
		~AnimEditor();

	public:
		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void Update(Starshine::GameTime& gameTime);
		void Draw(Starshine::GameTime& gameTime);

		i64 GetStateID() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};

		enum class DragAxis : i32
		{
			None = -1,

			Horizontal,
			Vertical
		};

		static constexpr f32 AxisToFavorThreshold{ 15.0f };

		struct DragStateData
		{
			bool BaseMousePositionSet{ false };
			bool FavorOneAxis{ false };

			vec2 BaseMousePosition{};

			vec2 AbsoluteMousePosition{};

			DragAxis AxisToFavor{ DragAxis::None };
			vec2 RelativeMousePosition{};

			vec2 DeltaMousePosition{};

			i32 HeldMouseButtonsMask{};
		} DragState;

		void ResetDragState();
	};
}
