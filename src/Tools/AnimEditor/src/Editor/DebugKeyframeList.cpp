#include "DebugKeyframeList.h"
#include "ImGui/Core/imgui.h"

using namespace Starshine::Graphics;
namespace Gui = ImGui;

namespace Starshine
{
	void DebugKeyframeList::SetAnimation(AnimationSet* animSet, Animation* anim)
	{
		currentAnimSet = animSet;
		currentAnim = anim;

		if (currentAnim != nullptr && !currentAnim->Layers.empty())
			layerToDisplay = &currentAnim->GetLayer(0);
	}

	void DebugKeyframeList::OnGUI()
	{
		if (!DrawWindow)
			return;

		if (Gui::Begin("Keyframe List", &DrawWindow))
		{
			Gui::Text("Layer");
			Gui::SameLine();
			if (Gui::BeginCombo("##LayerList", layerToDisplay != nullptr ? layerToDisplay->Name.c_str() : "[None]"))
			{
				for (Layer& layer : currentAnim->Layers)
				{
					const bool isSelected = layerToDisplay == &layer;
					if (Gui::Selectable(layer.Name.c_str(), isSelected))
						layerToDisplay = &layer;
				}

				Gui::EndCombo();
			}
			if (Gui::BeginTabBar("##AnimProps_TabBar"))
			{
				if (Gui::BeginTabItem("Origin"))
				{
					//if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Origin); }
					if (Gui::BeginTable("##FrameTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
					{
						Gui::TableSetupColumn("Frame");
						Gui::TableSetupColumn("Value");
						Gui::TableHeadersRow();
						if (layerToDisplay != nullptr)
						{
							for (const auto& keyframe : layerToDisplay->Origin)
							{
								Gui::TableNextRow();

								Gui::TableSetColumnIndex(0);
								Gui::Text("%d", keyframe.Frame);

								Gui::TableSetColumnIndex(1);
								Gui::Text("%.3f %.3f", keyframe.Value.x, keyframe.Value.y);
							}
						}

						Gui::EndTable();
					}
					Gui::EndTabItem();
				}
				if (Gui::BeginTabItem("Position"))
				{
					//if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Position); }
					if (Gui::BeginTable("##FrameTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
					{
						Gui::TableSetupColumn("Frame");
						Gui::TableSetupColumn("Value");
						Gui::TableHeadersRow();
						if (layerToDisplay != nullptr)
						{
							for (const auto& keyframe : layerToDisplay->Position)
							{
								Gui::TableNextRow();

								Gui::TableSetColumnIndex(0);
								Gui::Text("%d", keyframe.Frame);

								Gui::TableSetColumnIndex(1);
								Gui::Text("%.3f %.3f", keyframe.Value.x, keyframe.Value.y);
							}
						}

						Gui::EndTable();
					}
					Gui::EndTabItem();
				}
				if (Gui::BeginTabItem("Size"))
				{
					//if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Scale); }
					if (Gui::BeginTable("##FrameTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
					{
						Gui::TableSetupColumn("Frame");
						Gui::TableSetupColumn("Value");
						Gui::TableHeadersRow();
						if (layerToDisplay != nullptr)
						{
							for (const auto& keyframe : layerToDisplay->Scale)
							{
								Gui::TableNextRow();

								Gui::TableSetColumnIndex(0);
								Gui::Text("%d", keyframe.Frame);

								Gui::TableSetColumnIndex(1);
								Gui::Text("%.3f %.3f", keyframe.Value.x, keyframe.Value.y);
							}
						}

						Gui::EndTable();
					}
					Gui::EndTabItem();
				}
				if (Gui::BeginTabItem("Rotation"))
				{
					//if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Rotation); }
					if (Gui::BeginTable("##FrameTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
					{
						Gui::TableSetupColumn("Frame");
						Gui::TableSetupColumn("Value");
						Gui::TableHeadersRow();
						if (layerToDisplay != nullptr)
						{
							for (const auto& keyframe : layerToDisplay->Rotation)
							{
								Gui::TableNextRow();

								Gui::TableSetColumnIndex(0);
								Gui::Text("%d", keyframe.Frame);

								Gui::TableSetColumnIndex(1);
								Gui::Text("%.3f", keyframe.Value);
							}
						}

						Gui::EndTable();
					}
					Gui::EndTabItem();
				}
				if (Gui::BeginTabItem("Color"))
				{
					//if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Color); }
					if (Gui::BeginTable("##FrameTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
					{
						Gui::TableSetupColumn("Frame");
						Gui::TableSetupColumn("Value");
						Gui::TableHeadersRow();
						if (layerToDisplay != nullptr)
						{
							for (const auto& keyframe : layerToDisplay->Color)
							{
								Gui::TableNextRow();

								Gui::TableSetColumnIndex(0);
								Gui::Text("%d", keyframe.Frame);

								Gui::TableSetColumnIndex(1);
								Gui::Text("%d %d %d %d", keyframe.Value.R, keyframe.Value.G, keyframe.Value.B, keyframe.Value.A);
							}
						}

						Gui::EndTable();
					}
					Gui::EndTabItem();
				}
				Gui::EndTabBar();
			}

			Gui::End();
		}
	}
}
