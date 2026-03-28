#pragma once
#include "Animation.h"

namespace Sandbox::Animations
{
	f32 Interpolate(const KeyFrame* start, const KeyFrame* end, f32 frame);

	f32 GetValueAt(const std::vector<KeyFrame>& keyFrames, f32 frame);
	//f32 GetValueAt(const Property1D& property, f32 frame);
	//vec2 GetValueAt(const Property2D& property, f32 frame);
}
