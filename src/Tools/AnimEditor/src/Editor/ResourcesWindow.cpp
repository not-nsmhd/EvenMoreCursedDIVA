#include "ResourcesWindow.h"
#include "ImGui/Core/imgui.h"

using namespace Starshine::Graphics;
namespace Gui = ImGui;

namespace Starshine
{
	namespace
	{
		void SpriteDefContextMenu(Animation* anim, SpriteDefinition* spriteDef, std::vector<EditorLayer>& editorLayers)
		{
			if (Gui::BeginPopupContextItem())
			{
				if (Gui::Selectable("Add a new layer with this sprite"))
				{
					Layer& newLayer = anim->Layers.emplace_back();
					newLayer.Name = spriteDef->Name;
					newLayer.SpriteDefinition = spriteDef;
					newLayer.EndTime = anim->EndTime;

					EditorLayer editLayer = EditorLayer(anim->Layers, anim->Layers.size() - 1);
					editorLayers.push_back(editLayer);
				}

				ImGuiSelectableFlags flags = 0;
				if (spriteDef->RealSprite != nullptr)
					flags |= ImGuiSelectableFlags_Disabled;

				Gui::Selectable("Edit", false, flags);
				Gui::Selectable("Remove", false, flags);

				Gui::EndPopup();
			}
		}
	}

	ResourcesWindow::ResourcesWindow(EditorContextData* context) : context(context)
	{
	}

	void ResourcesWindow::OnGUI()
	{
		if (Gui::Begin("Resources"))
		{
			if (Gui::BeginTabBar("##Resources_TabBar"))
			{
				if (Gui::BeginTabItem("Animations"))
				{
					if (Gui::Button("+A"))
					{
						PrepareModalWindowData();
						newSize_common[0] = 0;
						newSize_common[1] = 60;
						Gui::OpenPopup("New Animation");
					}

					NewAnimationModalWindow();

					const ImVec2 contentRegion = Gui::GetContentRegionAvail();
					if (Gui::BeginListBox("##Resources_AnimationList", ImVec2(-FLT_MIN, contentRegion.y)))
					{
						for (auto& anim : context->AnimSet.GetAnimations())
						{
							const bool selected = &anim == context->CurrentAnimation;

							if (Gui::Selectable(anim.Name.c_str(), selected))
							{
								context->TimelineFrame = 0.0f;

								if (!selected)
								{
									context->CurrentAnimation = &anim;
									context->RecreateLayerList();
								}
							}
						}

						Gui::EndListBox();
					}
					Gui::EndTabItem();
				}
				if (Gui::BeginTabItem("Sprite Definitions"))
				{
					if (Gui::Button("+SD"))
					{
						PrepareModalWindowData();
						newSize_common[0] = 128;
						newSize_common[1] = 128;
						Gui::OpenPopup("New Sprite Definition");
					}

					NewSpriteDefinitionModalWindow();

					const ImVec2 contentRegion = Gui::GetContentRegionAvail();
					if (Gui::BeginListBox("##Resources_SpriteList", ImVec2(-FLT_MIN, contentRegion.y)))
					{
						for (auto& sprDef : context->AnimSet.GetSpriteDefinitions())
						{
							Gui::Selectable(sprDef.Name.c_str());
							SpriteDefContextMenu(context->CurrentAnimation, &sprDef, context->Layers);
						}

						Gui::EndListBox();
					}
					Gui::EndTabItem();
				}
				Gui::EndTabBar();
			}
		}
		Gui::End();
	}

	void ResourcesWindow::PrepareModalWindowData()
	{
		SDL_memset(newName_common, 0, sizeof(newName_common));
	}

	void ResourcesWindow::NewAnimationModalWindow()
	{
		const ImVec2 center = Gui::GetMainViewport()->GetCenter();
		Gui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (Gui::BeginPopupModal("New Animation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			Gui::Text("Name");
			Gui::SameLine();
			Gui::InputText("##NewAnimation_Name", newName_common, sizeof(newName_common) - 1);

			Gui::Text("Start and End Timings");
			Gui::SameLine();
			Gui::InputInt2("##NewAnimation_Timings", newSize_common);

			Gui::Separator();

			if (Gui::Button("Add", ImVec2(120.0f, 0.0f)))
			{
				Animation& newAnim = context->AnimSet.NewAnimation(newName_common, newSize_common[0], newSize_common[1]);
				context->CurrentAnimation = &newAnim;
				context->CurrentLayer = nullptr;
				context->CurrentLayerIndex = -1;
				Gui::CloseCurrentPopup();
			}

			Gui::SameLine();

			if (Gui::Button("Cancel", ImVec2(120.0f, 0.0f)))
				Gui::CloseCurrentPopup();

			Gui::EndPopup();
		}
	}

	void ResourcesWindow::NewSpriteDefinitionModalWindow()
	{
		const ImVec2 center = Gui::GetMainViewport()->GetCenter();
		Gui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (Gui::BeginPopupModal("New Sprite Definition", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			Gui::Text("Name");
			Gui::SameLine();
			Gui::InputText("##NewSpriteDef_Name", newName_common, sizeof(newName_common) - 1);

			Gui::Text("Size");
			Gui::SameLine();
			Gui::InputInt2("##NewSpriteDef_Size", newSize_common);

			Gui::Separator();

			if (Gui::Button("Add", ImVec2(120.0f, 0.0f)))
			{
				SpriteDefinition& sprDef = context->AnimSet.NewSpriteDefinition(newName_common, vec2(newSize_common[0], newSize_common[1]), nullptr);
				Gui::CloseCurrentPopup();
			}

			Gui::SameLine();

			if (Gui::Button("Cancel", ImVec2(120.0f, 0.0f)))
				Gui::CloseCurrentPopup();

			Gui::EndPopup();
		}
	}
}
