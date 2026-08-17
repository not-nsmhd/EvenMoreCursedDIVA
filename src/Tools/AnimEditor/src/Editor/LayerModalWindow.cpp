#include "LayerModalWindow.h"
#include "ImGui/Core/imgui.h"

namespace Gui = ImGui;
using namespace Starshine::Graphics;

namespace Starshine
{
	enum class PopupType : i32
	{
		None,
		Rename,
		ChangeSprite,
		Remove
	};

	PopupType openPopupType{};

	LayerModalWindow::LayerModalWindow(EditorContextData* context) : context(context)
	{
	}

	void LayerModalWindow::OpenRename(i32 layerIndex)
	{
		layerToModify = context->Layers[layerIndex].GetLayerPointer();
		SDL_memset(newLayerName, 0, sizeof(newLayerName));

		openPopupType = PopupType::Rename;
	}

	void LayerModalWindow::OpenChangeSprite(i32 layerIndex)
	{
		layerToModify = context->Layers[layerIndex].GetLayerPointer();
		openPopupType = PopupType::ChangeSprite;
	}

	void LayerModalWindow::OpenRemoveLayer(i32 index)
	{
		layerToRemoveIndex = index;
		openPopupType = PopupType::Remove;
	}

	void LayerModalWindow::RenameLayerWindow()
	{
		if (layerToModify == nullptr)
			return;

		const ImVec2 center = Gui::GetMainViewport()->GetCenter();
		Gui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (Gui::Begin("Rename Layer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			Gui::InputText("##RenameLayer_Name", newLayerName, sizeof(newLayerName));

			Gui::Separator();

			if (Gui::Button("Rename", ImVec2(120.0f, 0.0f)))
			{
				layerToModify->Name = newLayerName;
				openPopupType = PopupType::None;
				layerToModify = nullptr;
			}

			Gui::SameLine();

			if (Gui::Button("Cancel", ImVec2(120.0f, 0.0f)))
			{
				openPopupType = PopupType::None;
				layerToModify = nullptr;
			}

			Gui::End();
		}
	}

	void LayerModalWindow::ChangeLayerSpriteWindow()
	{
		if (layerToModify == nullptr)
			return;

		const ImVec2 center = Gui::GetMainViewport()->GetCenter();
		Gui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (Gui::Begin("Change Layer Sprite", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const f32 listHeight = Gui::GetTextLineHeightWithSpacing() * 8.0f;
			if (Gui::BeginListBox("##ChangeLayerSprite_List", ImVec2(-FLT_MIN, listHeight)))
			{
				for (auto& sprite : context->AnimSet.GetSpriteDefinitions())
				{
					const bool selected = spriteToSet == &sprite;
					if (Gui::Selectable(sprite.Name.c_str(), selected))
						spriteToSet = &sprite;
				}

				Gui::EndListBox();
			}

			Gui::Separator();

			if (Gui::Button("Change Sprite", ImVec2(120.0f, 0.0f)))
			{
				layerToModify->SpriteDefinition = spriteToSet;
				openPopupType = PopupType::None;
				layerToModify = nullptr;
			}

			Gui::SameLine();

			if (Gui::Button("Cancel", ImVec2(120.0f, 0.0f)))
			{
				openPopupType = PopupType::None;
				layerToModify = nullptr;
			}

			Gui::End();
		}
	}

	void LayerModalWindow::RemoveLayerWindow()
	{
		if (layerToRemoveIndex == -1)
			return;

		const ImVec2 center = Gui::GetMainViewport()->GetCenter();
		Gui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (Gui::Begin("Remove Layer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			EditorLayer* layer = &context->Layers[layerToRemoveIndex];

			Gui::Text("Are you sure you want to remove layer\n\"%s\"?", layer->BaseLayer->Name.c_str());

			Gui::Separator();

			if (Gui::Button("Yes", ImVec2(120.0f, 0.0f)))
			{
				auto& layerList = context->CurrentAnimation->Layers;
				layerList.erase(layerList.begin() + layerToRemoveIndex);

				context->Layers.erase(context->Layers.begin() + layerToRemoveIndex);

				i32 layerIndex = 0;
				for (auto& layer : context->Layers)
					layer.BaseLayer.Index = layerIndex++; // editor layer list is always sequential, so whatever

				openPopupType = PopupType::None;
				layerToRemoveIndex = -1;
			}

			Gui::SameLine();

			if (Gui::Button("No", ImVec2(120.0f, 0.0f)))
			{
				openPopupType = PopupType::None;
				layerToRemoveIndex = -1;
			}

			Gui::End();
		}
	}

	void LayerModalWindow::OnGUI()
	{
		switch (openPopupType)
		{
		case PopupType::Rename:
			RenameLayerWindow();
			break;
		case PopupType::ChangeSprite:
			ChangeLayerSpriteWindow();
			break;
		case PopupType::Remove:
			RemoveLayerWindow();
			break;
		}
	}
}
