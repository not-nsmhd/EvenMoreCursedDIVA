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

		inline std::string_view GetStateName() { return "AnimEditor"; };

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

			struct UserBaseValuesData
			{
				union
				{
					u8 Bytes[32]{};
					ivec4 Intergers;
					vec4 Floats;
				};
				bool BaseValuesSet{ false };
			} UserBaseValues;

			// TODO: Test both this assert and the assert below it with different (gcc and/or clang) compilers
			static_assert(alignof(UserBaseValuesData) == 4);
			// NOTE: Extra 3 bytes are needed due to struct alignment (4 bytes)
			static_assert(sizeof(UserBaseValuesData) == 36);
		} DragState;

		void ResetDragState();
	};
}
