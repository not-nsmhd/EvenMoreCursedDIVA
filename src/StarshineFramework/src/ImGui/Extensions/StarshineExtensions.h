#pragma once
#include "Common/Types.h"
#include "ImGui/Core/imgui.h"
#include "Rendering/Render2D/SpriteSheet.h"

namespace Starshine::ImGuiExtensions
{
	inline ImU32 GetU32StyleColor(const ImGuiCol colorIndex)
	{
		ImVec4* styleColors = ImGui::GetStyle().Colors;
		return IM_COL32(styleColors[colorIndex].x * 255, styleColors[colorIndex].y * 255, styleColors[colorIndex].z * 255, styleColors[colorIndex].w * 255);
	}
}
