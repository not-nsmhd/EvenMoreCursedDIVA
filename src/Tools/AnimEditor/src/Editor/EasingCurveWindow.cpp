#include "EasingCurveWindow.h"
#include "ImGui/Core/imgui.h"
#include "ImGui/Core/imgui_internal.h"

using namespace Starshine::Graphics;
namespace Gui = ImGui;

namespace Starshine
{
	static constexpr i32 GraphResolution = 64;

	namespace Detail
	{
		f32 getGraphValue(void* easingPoints_v, i32 idx)
		{
			vec2** easingPoints = static_cast<vec2**>(easingPoints_v);
			f32 t = static_cast<f32>(idx) / static_cast<f32>(GraphResolution);
			return MathExtensions::GetEasingPoint(*easingPoints[0], *easingPoints[1], t);
		}
	}

	void EasingCurveWindow::SetEasingPoints(vec2* point1, vec2* point2)
	{
		easingPointsToModify[0] = point1;
		easingPointsToModify[1] = point2;
	}

	void EasingCurveWindow::OnGUI()
	{
		if (!DrawWindow)
			return;

		if (Gui::Begin("Easing Curve", &DrawWindow))
		{
			if (easingPointsToModify[0] != nullptr && easingPointsToModify[1] != nullptr)
			{
				Gui::Text("Start");
				Gui::SameLine();
				Gui::SetNextItemWidth(64.0f);
				Gui::DragFloat("##EasingCurve_Start_X", &easingPointsToModify[0]->x, 0.01f, 0.0f, 1.0f);
				Gui::SameLine();
				Gui::SetNextItemWidth(64.0f);
				Gui::DragFloat("##EasingCurve_Start_Y", &easingPointsToModify[0]->y, 0.01f, 0.0f, 1.0f);
				Gui::SameLine();
				if (Gui::Button("Reset##Start"))
					*easingPointsToModify[0] = LinearEasing;

				Gui::Text("End");
				Gui::SameLine();
				Gui::SetNextItemWidth(64.0f);
				Gui::DragFloat("##EasingCurve_End_X", &easingPointsToModify[1]->x, 0.01f, 0.0f, 1.0f);
				Gui::SameLine();
				Gui::SetNextItemWidth(64.0f);
				Gui::DragFloat("##EasingCurve_End_Y", &easingPointsToModify[1]->y, 0.01f, 0.0f, 1.0f);
				Gui::SameLine();
				if (Gui::Button("Reset##End"))
					*easingPointsToModify[1] = LinearEasing;

				Gui::Text("Graph");
				const ImVec2 contentRegion = Gui::GetContentRegionAvail();
				Gui::PlotLines("##EasingCurve_Graph", Detail::getGraphValue, easingPointsToModify, GraphResolution, 0, nullptr, 0.0f, 1.0f, contentRegion);
			}

			Gui::End();
		}
	}
}
