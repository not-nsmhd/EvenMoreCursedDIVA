#include "SpriteEditorWindow.h"
#include "ImGui/Core/imgui.h"
#include "ImGui/Core/imgui_internal.h"
#include <Rendering/D3D11/D3D11Texture.h>
#include <Rendering/Utilities.h>
#include "FileDialog.h"
#include <IO/Path/Path.h>
#include <IO/Path/Directory.h>
#include <IO/Xml.h>

using namespace Starshine::Graphics;
using namespace Starshine::Rendering::D3D11;
using namespace Starshine::IO;
namespace Gui = ImGui;

namespace Starshine
{
	void SpriteEditorWindow::SetSpriteSheet(Graphics::SpriteSheet* spriteSheet)
	{
		currentSpriteSheet = spriteSheet;
		sprites.clear();

		sprites.reserve(spriteSheet->GetSpriteCount());
		for (auto& sprite : currentSpriteSheet->GetSprites())
		{
			EditorSprite& sprEx = sprites.emplace_back();
			sprEx.BaseSprite = &sprite;
			sprEx.RealSize = sprite.SourceRectangle.Size();
		}

		currentSprite = &sprites.front();
	}

	void SpriteEditorWindow::SetCurrentSprite(Graphics::Sprite* sprite)
	{
		//currentSprite = sprite;
	}

	void SpriteEditorWindow::OnGUI()
	{
		if (!DrawWindow)
			return;

		if (Gui::Begin("Sprite Editor", &DrawWindow, ImGuiWindowFlags_MenuBar))
		{
			WindowMenuBar();

			Gui::Text("Sprite");
			const char* previewText = (currentSprite != nullptr ? currentSprite->BaseSprite->Name.c_str() : "[None]");

			if (Gui::BeginCombo("##SpriteEditor_List", previewText))
			{
				for (auto& sprite : sprites)
				{
					if (Gui::Selectable(sprite.BaseSprite->Name.c_str()))
						currentSprite = &sprite;
				}

				Gui::EndCombo();
			}

			if (currentSpriteSheet == nullptr || currentSprite == nullptr)
			{
				Gui::End();
				return;
			}

			vec2 sprSize = currentSprite->BaseSprite->SourceRectangle.Size();
			vec2 shiftedSize = currentSprite->RealSize;

			Gui::Text("Origin");
			Gui::SameLine();
			Gui::SetNextItemWidth(64.0f);
			Gui::DragFloat("##SpriteEditor_Origin_X", &currentSprite->BaseSprite->Origin.x, 0.5f, 0.0f, sprSize.x);
			Gui::SameLine();
			Gui::SetNextItemWidth(64.0f);
			Gui::DragFloat("##SpriteEditor_Origin_Y", &currentSprite->BaseSprite->Origin.y, 0.5f, 0.0f, sprSize.y);
			Gui::SameLine();
			if (Gui::Button("Center"))
				currentSprite->BaseSprite->Origin = currentSprite->RealSize / 2.0f;

			Gui::Text("Source Shift");
			Gui::SameLine();
			Gui::SetNextItemWidth(64.0f);
			Gui::DragFloat("##SpriteEditor_SrcShift_X", &currentSprite->SourceShift.x, 0.5f, 0.0f, sprSize.x);
			Gui::SameLine();
			Gui::SetNextItemWidth(64.0f);
			Gui::DragFloat("##SpriteEditor_SrcShift_Y", &currentSprite->SourceShift.y, 0.5f, 0.0f, sprSize.y);
			Gui::SameLine();
			if (Gui::Button("Reset##SourceShift"))
				currentSprite->SourceShift = {};

			Gui::Text("Size");
			Gui::SameLine();
			Gui::SetNextItemWidth(64.0f);
			Gui::DragFloat("##SpriteEditor_SprSize_X", &currentSprite->RealSize.x, 0.5f, 1.0f, sprSize.x);
			Gui::SameLine();
			Gui::SetNextItemWidth(64.0f);
			Gui::DragFloat("##SpriteEditor_SprSize_Y", &currentSprite->RealSize.y, 0.5f, 1.0f, sprSize.y);
			Gui::SameLine();
			if (Gui::Button("Reset##SprSize"))
				currentSprite->RealSize = currentSprite->BaseSprite->SourceRectangle.Size();

			Gui::Text("Display Scale");
			Gui::SameLine();
			Gui::SetNextItemWidth(100.0f);
			Gui::SameLine();
			Gui::SetNextItemWidth(64.0f);
			Gui::DragFloat("##SpriteEditor_DisplayScale_X", &displayScale.x, 1.0f, 1.0f);
			Gui::SameLine();
			Gui::SetNextItemWidth(64.0f);
			Gui::DragFloat("##SpriteEditor_DisplayScale_Y", &displayScale.y, 1.0f, 1.0f);

			Texture* sprTex = currentSpriteSheet->GetTexture(currentSprite->BaseSprite->TextureIndex);
			Rendering::Utilities::EnsureTextureIsUploaded(sprTex);

			const ivec2 texSize = sprTex->GetSize();

			const D3D11Texture* d3dTex = static_cast<const D3D11Texture*>(sprTex->GPUTexture.Resource.get());

			ImDrawList* drawList = Gui::GetWindowDrawList();

			sprSize *= displayScale;
			shiftedSize *= displayScale;

			const ImVec2 windowPos = Gui::GetWindowPos();
			ImVec2 cursorPos(Gui::GetCursorPosX() + windowPos.x, Gui::GetCursorPosY() + windowPos.y);

			const ImRect imageRect(cursorPos.x, cursorPos.y, cursorPos.x + sprSize.x, cursorPos.y + sprSize.y);

			const vec2 shiftedPos = currentSprite->SourceShift * displayScale;
			const ImRect shiftedRect(cursorPos.x + shiftedPos.x, cursorPos.y + shiftedPos.y,
				cursorPos.x + shiftedSize.x, cursorPos.y + shiftedSize.y);

			const ImVec2 originPoint(cursorPos.x + currentSprite->BaseSprite->Origin.x * displayScale.x,
				cursorPos.y + currentSprite->BaseSprite->Origin.y * displayScale.y);

			if (d3dTex != nullptr)
			{
				const ImVec2 sprUV1(currentSprite->BaseSprite->SourceRectangle.X / texSize.x, currentSprite->BaseSprite->SourceRectangle.Y / texSize.y);
				const ImVec2 sprUV2((currentSprite->BaseSprite->SourceRectangle.X + currentSprite->BaseSprite->SourceRectangle.Width) / texSize.x,
					(currentSprite->BaseSprite->SourceRectangle.Y + currentSprite->BaseSprite->SourceRectangle.Height) / texSize.y);

				const ImU32 redColor = IM_COL32(255, 0, 0, 255);

				drawList->AddImage(d3dTex->ShaderResourceView.Get(), imageRect.GetTL(), imageRect.GetBR(), sprUV1, sprUV2);
				drawList->AddRect(shiftedRect.GetTL(), shiftedRect.GetBR(), redColor);
				drawList->AddRect(imageRect.GetTL(), imageRect.GetBR(), IM_COL32_WHITE);
				drawList->AddCircleFilled(originPoint, 5.0f, redColor, 4);
				drawList->AddCircle(originPoint, 5.0f, IM_COL32_WHITE, 4);
			}
		}
		Gui::End();
	}

	void SpriteEditorWindow::WindowMenuBar()
	{
		if (Gui::BeginMenuBar())
		{
			if (Gui::BeginMenu("Export"))
			{
				if (Gui::MenuItem("Selected Sprite Properties"))
				{
					FileDialog dialog;
					dialog.Title = "Export selected sprite properties";

					if (dialog.OpenDirectory())
						ExportSpritePropertyFile(dialog.OutputFilePath, *currentSprite);
				}
				if (Gui::MenuItem("All Sprite Properties"))
				{
					FileDialog dialog;
					dialog.Title = "Export all sprite properties";
					
					if (dialog.OpenDirectory())
						ExportSpriteProperties(dialog.OutputFilePath);
				}

				Gui::EndMenu();
			}

			Gui::EndMenuBar();
		}
	}

	void SpriteEditorWindow::ExportSpritePropertyFile(std::string_view dirPath, const EditorSprite& sprite)
	{
		std::string filePath = Path::Append(dirPath, sprite.BaseSprite->Name);
		filePath = Path::ChangeExtension(filePath, ".xml");

		Xml::Document document;
		Xml::Element* rootElement = document.NewElement("SpritePackOptions");

		rootElement->SetAttribute("RealSourceX", sprite.SourceShift.x);
		rootElement->SetAttribute("RealSourceY", sprite.SourceShift.y);
		rootElement->SetAttribute("RealWidth", sprite.RealSize.x);
		rootElement->SetAttribute("RealHeight", sprite.RealSize.y);
		rootElement->SetAttribute("OriginX", sprite.BaseSprite->Origin.x);
		rootElement->SetAttribute("OriginY", sprite.BaseSprite->Origin.y);
		rootElement->SetAttribute("DesiredTextureIndex", sprite.BaseSprite->TextureIndex);

		document.InsertFirstChild(rootElement);
		document.SaveFile(filePath.data());
	}

	void SpriteEditorWindow::ExportSpriteProperties(std::string_view dirPath)
	{
		if (!Directory::Exists(dirPath))
			return;

		for (const auto& sprite : sprites)
			ExportSpritePropertyFile(dirPath, sprite);
	}
}
