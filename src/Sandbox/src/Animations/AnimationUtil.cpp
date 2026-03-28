#include "AnimationUtil.h"
#include "Common/MathExt.h"

using namespace Starshine;

namespace Sandbox::Animations
{
	f32 Interpolate(const KeyFrame* start, const KeyFrame* end, f32 frame)
	{
		if (start == nullptr) { return 0.0f; }
		if (end == nullptr || start == end) { return start->Value; }

		f32 f_normalized = MathExtensions::ConvertRange<f32>(start->Frame, end->Frame, 0.0f, 1.0f, frame);

		f32 b_start = MathExtensions::Lerp<f32>(0.0f, start->BiasStart, f_normalized);
		f32 b_mid = MathExtensions::Lerp<f32>(start->BiasStart, start->BiasEnd, f_normalized);
		f32 b_end = MathExtensions::Lerp<f32>(start->BiasEnd, 1.0f, f_normalized);

		f32 t_startToMid = MathExtensions::Lerp<f32>(b_start, b_mid, f_normalized);
		f32 t_midToEnd = MathExtensions::Lerp<f32>(b_mid, b_end, f_normalized);

		f32 t = MathExtensions::Lerp<f32>(t_startToMid, t_midToEnd, f_normalized);
		t = MathExtensions::Clamp(t, 0.0f, 1.0f);

		return MathExtensions::Lerp<f32>(start->Value, end->Value, t);
	}

	f32 GetValueAt(const std::vector<KeyFrame>& keyFrames, f32 frame)
	{
		if (keyFrames.size() == 0) { return 0.0f; }
		else if (keyFrames.size() == 1) { return keyFrames[0].Value; }

		u32 wholeFramePart = static_cast<u32>(std::floorf(frame));

		const KeyFrame* start = &keyFrames[0];
		const KeyFrame* end = &keyFrames[0];

		for (size_t i = 0; i < keyFrames.size(); i++)
		{
			end = &keyFrames[i];
			if (end->Frame >= wholeFramePart) break;

			start = end;
		}

		return Interpolate(start, end, frame);
	}
}
