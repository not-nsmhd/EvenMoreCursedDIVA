#pragma once
#include <Common/Types.h>
#include "EditorContext.h"

namespace Starshine
{
	class EasingCurveWindow : NonCopyable
	{
	public:
		EasingCurveWindow() = default;
		~EasingCurveWindow() = default;

	public:
		void SetEasingPoints(vec2* point1, vec2* point2);

		void OnGUI();

	public:
		bool DrawWindow{};

	private:
		vec2* easingPointsToModify[2]{};
	};
}
