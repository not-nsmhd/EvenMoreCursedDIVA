#pragma once
#include "Common/Types.h"
#include "Common/Color.h"
#include <vector>

namespace Sandbox::Animations
{
	struct KeyFrame
	{
		u32 Frame{};
		f32 Value{};

		f32 BiasStart{};
		f32 BiasEnd{};

		constexpr KeyFrame(u32 frame, f32 value)
			: Frame(frame), Value(value), BiasStart(0.25f), BiasEnd(0.75f) {};

		constexpr KeyFrame(u32 frame, f32 value, f32 biasStart, f32 biasEnd)
			: Frame(frame), Value(value), BiasStart(biasStart), BiasEnd(biasEnd) {};
	};

	struct KeyFrameColor
	{
		u32 Frame{};
		Starshine::Color Value{};

		constexpr KeyFrameColor(u32 frame, const Starshine::Color& value) : Frame(frame), Value(value) {};
	};

	struct Property1D
	{
		std::vector<KeyFrame> KeyFrames;

		inline std::vector<KeyFrame>* operator->() { return &KeyFrames; }
		inline const std::vector<KeyFrame>* operator->() const { return &KeyFrames; }
	};

	struct Property2D
	{
		Property1D X, Y;
	};

	struct PropertyColor
	{
		std::vector<KeyFrameColor> KeyFrames;

		inline std::vector<KeyFrameColor>* operator->() { return &KeyFrames; }
		inline const std::vector<KeyFrameColor>* operator->() const { return &KeyFrames; }
	};

	enum class AnimatedObjectProperty
	{
		OriginX,
		OriginY,
		PositionX,
		PositionY,
		SizeX,
		SizeY,
		Rotation,
		Color
	};

	struct AnimatedObject
	{
		Property2D Origin;
		Property2D Position;
		Property2D Size;
		Property1D Rotation;
		PropertyColor Color;
	};
}
