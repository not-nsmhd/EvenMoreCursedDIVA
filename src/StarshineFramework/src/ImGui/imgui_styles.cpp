#include "imgui_styles.h"
#include "imgui.h"

namespace Starshine
{
	void ImGuiStyles::ApplyImGuiStyle()
	{
		// Photoshop Style from https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.28f, 0.28f, 0.28f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.36f, 0.36f, 0.36f, 0.60f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(1.00f, 1.00f, 1.00f, 0.12f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
		colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
		colors[ImGuiCol_Header] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		colors[ImGuiCol_TabSelected] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_TabDimmed] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
		colors[ImGuiCol_NavCursor] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.39f, 0.00f, 1.00f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.59f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.59f);
	}
}
