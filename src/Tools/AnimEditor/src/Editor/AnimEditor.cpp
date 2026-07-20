#include "AnimEditor.h"
#include "Definitions.h"
#include <Rendering/Device.h>
#include <Rendering/Render2D/SpriteSheet.h>
#include <GameContext.h>
#include <Common/MathExt.h>
#include <Input/Mouse.h>

#include <ImGui/Core/imgui.h>
#include <ImGui/Core/imgui_internal.h>

namespace Gui = ImGui;
using namespace Starshine::Rendering::Render2D;

namespace Starshine
{
	struct Transform2D
	{
		vec2 Origin{};
		vec2 Position{};
		vec2 Size{};
		f32 Rotation{};
		Color Color{};
	};

	template <typename T>
	struct Keyframe
	{
		f32 Frame{};
		T Value{};
	};

	struct Layer
	{
		std::string Name;

		f32 StartTime{};
		f32 Duration{};

		std::vector<Keyframe<vec2>> Origin;
		std::vector<Keyframe<vec2>> Position;
		std::vector<Keyframe<vec2>> Size;
		std::vector<Keyframe<f32>> Rotation;
		std::vector<Keyframe<Color>> Color;
		const Sprite* Sprite{};

		Transform2D CurrentEditTransform{};
		bool Expanded{};
	};

	ImRect GetRelativeContentRegion()
	{
		const ImVec2 cursorPos = Gui::GetCursorScreenPos();
		const ImVec2 contentRegion = Gui::GetContentRegionAvail();

		return ImRect(cursorPos.x, cursorPos.y, cursorPos.x + contentRegion.x, cursorPos.y + contentRegion.y);
	}

	struct AnimEditor::Impl
	{
		GFX::SpritePacker sprPacker{};
		SpriteSheet spriteSheet{};

		std::vector<Layer> layers{};
		Layer* selectedLayer{ nullptr };

		f32 timelineFrame{};
		f32 animLength{ 60.0f };

		ivec2 canvasSize{ 1280, 720 };
		vec2 viewPan{};
		f32 viewZoom{ 1.0f };

		bool playing = false;
		bool showMetricsWindow = false;

		Layer* layerToModify = nullptr;
		bool renameLayer = false;
		bool changeLayerSprite = false;

		std::vector<Layer>::iterator layerToDelete{ layers.begin() };
		bool layerToDeleteIsSet = false;

		Impl()
		{
		}

		~Impl()
		{
		}

		bool Initialize()
		{
			const RectangleF viewportSize = Rendering::GetDevice()->GetViewportSize();

			viewPan.x = viewportSize.Width / 2.0f - canvasSize.x / 2.0f;
			viewPan.y = viewportSize.Height / 2.0f - canvasSize.y / 2.0f;
			return true;
		}

		bool LoadContent()
		{
			sprPacker.AddFromDirectory("AnimEditor/sprites/devtest");
			sprPacker.Pack();

			spriteSheet.CreateFromSpritePacker(sprPacker);

			sprPacker.Clear();
			return true;
		}

		template<typename T>
		void InsertKeyframe(std::vector<Keyframe<T>>& keyframes, const f32& frame, const T& value)
		{
			if (keyframes.size() == 0)
			{
				Keyframe<T>& keyframe = keyframes.emplace_back();

				keyframe.Frame = frame;
				keyframe.Value = value;
				return;
			}

			size_t kfCount = keyframes.size();
			auto placement = keyframes.begin();

			for (size_t i = 0; i < kfCount; i++, placement++)
			{
				if (i == kfCount) { break; }

				Keyframe& kf = keyframes[i];
				if ((i32)kf.Frame == (i32)frame)
				{
					kf.Value = value;
					return;
				}
				if (frame > kf.Frame)
				{
					continue;
				}

				keyframes.emplace(placement, Keyframe{ frame, value });
				return;
			}

			keyframes.push_back({ frame, value });
		}

		void TimelineHeader()
		{
			Gui::Text("Frame");
			Gui::SameLine();
			Gui::SetNextItemWidth(56.0f);
			Gui::DragFloat("##Timeline_FrameDrag", &timelineFrame, 1.0f, 0.0f, animLength - 1.0f, "%.0f", playing ? ImGuiSliderFlags_ReadOnly : 0);
			/*if (Gui::IsItemEdited() && keyframes.size() > 0)
			{
				currentValue = GetKeyframeValue(timelineFrame);
			}

			Gui::SameLine();
			if (Gui::Button("Play"))
			{
				if (!keyframes.empty()) { playing = !playing; }
				if (!playing)
				{
					drawPos = currentValue;
				}
			}*/
		}

		void DrawLayerProperty_Begin(i32 layerIndex, const char* propName, char* idBuffer)
		{
			Gui::Text(propName);

			Gui::SameLine();
			Gui::SetNextItemWidth(192.0f);
			SDL_snprintf(idBuffer, sizeof(idBuffer) - 1, "##L%d_%s_Drag", layerIndex, propName);
		}

		void DrawLayerProperty_End(i32 layerIndex, const char* propName, char* idBuffer)
		{
			SDL_snprintf(idBuffer, sizeof(idBuffer) - 1, "##L%d_%s_AddFrame", layerIndex, propName);

			Gui::SameLine();
			Gui::PushID(idBuffer);
			if (Gui::Button("+F")) {}
			Gui::PopID();
		}

		void DrawLayerPropertyField_Vec2(i32 layerIndex, std::string_view propName, vec2& value)
		{
			const char* propName_data = propName.data();
			char idBuffer[64]{};

			DrawLayerProperty_Begin(layerIndex, propName_data, idBuffer);
			Gui::DragFloat2(idBuffer, &value.x);
			DrawLayerProperty_End(layerIndex, propName_data, idBuffer);
		}

		void DrawLayerPropertyField_F32(i32 layerIndex, std::string_view propName, f32& value)
		{
			const char* propName_data = propName.data();
			char idBuffer[64]{};

			DrawLayerProperty_Begin(layerIndex, propName_data, idBuffer);
			Gui::DragFloat(idBuffer, &value);
			DrawLayerProperty_End(layerIndex, propName_data, idBuffer);
		}

		void DrawLayerPropertyField_Color(i32 layerIndex, std::string_view propName, Color& value)
		{
			const char* propName_data = propName.data();
			char idBuffer[64]{};

			DrawLayerProperty_Begin(layerIndex, propName_data, idBuffer);

			static vec4 colorTemp = value.ToVector4();
			Gui::ColorEdit4(idBuffer, &colorTemp.r);
			value = Color(colorTemp);

			DrawLayerProperty_End(layerIndex, propName_data, idBuffer);
		}

		void LayerContextMenu(size_t layerIndex)
		{
			if (Gui::BeginPopupContextItem())
			{
				auto layer = layers.begin() + layerIndex;
				if (Gui::Selectable("Rename"))
				{
					layerToModify = &*layer;
					renameLayer = true;
				}
				if (Gui::Selectable("Change Sprite"))
				{
					layerToModify = &*layer;
					changeLayerSprite = true;
				}

				Gui::Separator();

				if (Gui::Selectable("Position at center"))
				{
					layer->CurrentEditTransform.Position.x = canvasSize.x / 2.0f;
					layer->CurrentEditTransform.Position.y = canvasSize.y / 2.0f;
				}

				if (Gui::BeginMenu("Set layer's origin..."))
				{
					if (Gui::Selectable("at sprite's origin"))
					{
						layer->CurrentEditTransform.Origin = layer->Sprite->Origin;
					}
					if (Gui::Selectable("at sprite's center"))
					{
						layer->CurrentEditTransform.Origin = layer->Sprite->SourceRectangle.Center();
					}
					if (Gui::Selectable("at layer's center"))
					{
						layer->CurrentEditTransform.Origin = layer->CurrentEditTransform.Size / 2.0f;
					}
					Gui::EndMenu();
				}

				Gui::Separator();

				if (Gui::Selectable("Delete"))
				{
					layerToDelete = layer;
					layerToDeleteIsSet = true;
				}
				Gui::EndPopup();
			}
		}

		void LayerList()
		{
			const ImRect dopeSheetRegion = GetRelativeContentRegion();

			ImVec2 layersRegionSize = dopeSheetRegion.GetBR();
			layersRegionSize.x *= 0.2f;
			layersRegionSize.y = Gui::GetContentRegionAvail().y;

			if (Gui::BeginChild("##Timeline_DopeSheet_Layers", layersRegionSize,
				ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX, ImGuiWindowFlags_MenuBar))
			{
				if (Gui::BeginMenuBar())
				{
					if (Gui::MenuItem("+L"))
					{
						size_t layerCount = layers.size();
						char layerName[64]{};
						SDL_snprintf(layerName, sizeof(layerName) - 1, "Layer %llu", layerCount);

						const Sprite& defaultSprite = spriteSheet.GetSprite(0);
						Layer& newLayer = layers.emplace_back();
						newLayer.Name = layerName;
						newLayer.Sprite = &defaultSprite;
						newLayer.CurrentEditTransform.Size = vec2(defaultSprite.SourceRectangle.Width, defaultSprite.SourceRectangle.Height);
						newLayer.CurrentEditTransform.Origin = defaultSprite.Origin;
						newLayer.CurrentEditTransform.Color = DefaultColors::White;
					}
					if (Gui::BeginItemTooltip())
					{
						Gui::Text("Add a new layer");
						Gui::EndTooltip();
					}
				}
				Gui::EndMenuBar();

				i32 layerIndex = 0;

				for (auto layer = layers.begin(); layer != layers.end(); layer++)
				{
					Gui::PushID(layerIndex);
					if (Gui::TreeNode("", layer->Name.c_str()))
					{
						layer->Expanded = true;

						DrawLayerPropertyField_Vec2(layerIndex, "Origin", layer->CurrentEditTransform.Origin);
						DrawLayerPropertyField_Vec2(layerIndex, "Position", layer->CurrentEditTransform.Position);
						DrawLayerPropertyField_Vec2(layerIndex, "Size", layer->CurrentEditTransform.Size);
						DrawLayerPropertyField_F32(layerIndex, "Rotation", layer->CurrentEditTransform.Rotation);
						DrawLayerPropertyField_Color(layerIndex, "Color", layer->CurrentEditTransform.Color);
						Gui::TreePop();
					}
					Gui::PopID();

					LayerContextMenu(layerIndex);
					layerIndex++;
				}
			}
			Gui::EndChild();
		}

		static constexpr f32 frameLineDistance = 15.0f;

		template <typename T>
		void DrawKeyframes(const std::vector<Keyframe<T>>& frames, const f32& xPos, const f32& yPos, ImDrawList* drawList)
		{
			const ImGuiStyle& style = Gui::GetStyle();

			for (auto& keyframe : frames)
			{
				const ImVec2 framePos = { xPos + keyframe.Frame * frameLineDistance, yPos };

				drawList->AddCircleFilled(framePos, 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLines]), 4);
			}
		}

		void DrawTimeline()
		{
			if (Gui::Begin("Timeline"))
			{
				ImGuiIO& io = Gui::GetIO();
				const ImGuiStyle& style = Gui::GetStyle();

				const ImVec2 contentRegion = Gui::GetContentRegionAvail();

				// NOTE: https://github.com/ocornut/imgui/issues/3284#issuecomment-641397151
				const f32 menuBarHeight = style.FontSizeBase + (style.FramePadding.y * 2.0f);
				const ImVec2 padding = style.FramePadding;

				TimelineHeader();
				LayerList();

				// --- Timeline (keyframes)
				ImGui::SameLine();

				const ImRect timelineRegion = GetRelativeContentRegion();
				if (Gui::BeginChild("##Timeline_DopeSheet_Keyframes", {}, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_HorizontalScrollbar))
				{
					ImDrawList* drawList = Gui::GetWindowDrawList();
					drawList->PushClipRect(timelineRegion.GetTL(), timelineRegion.GetBR());

					f32 scroll = Gui::GetScrollX();

					const ImRect frameNumbersRegion = ImRect{ timelineRegion.Min.x, timelineRegion.Min.y, timelineRegion.Max.x, timelineRegion.Min.y + menuBarHeight };
					const ImRect keyframesRegion = ImRect{ timelineRegion.Min.x, timelineRegion.Min.y + menuBarHeight, timelineRegion.Max.x, timelineRegion.Max.y - style.ScrollbarSize };

					// --- Frame lines + numbers (top bar)
					drawList->AddRectFilled(keyframesRegion.GetTL(), keyframesRegion.GetBR(), Gui::GetColorU32(style.Colors[ImGuiCol_FrameBg]));
					drawList->AddRectFilled(frameNumbersRegion.GetTL(), frameNumbersRegion.GetBR(), Gui::GetColorU32(style.Colors[ImGuiCol_Button]));
					drawList->AddRect(timelineRegion.GetTL(), timelineRegion.GetBR(), Gui::GetColorU32(style.Colors[ImGuiCol_Border]));

					for (i32 i = 0; i < (i32)animLength; i++)
					{
						const ImVec2 lineStart{ frameNumbersRegion.Min.x + i * frameLineDistance + padding.x - scroll, frameNumbersRegion.Max.y };
						const ImVec2 lineEnd{ lineStart.x, keyframesRegion.Max.y };

						if (i % 5 == 0)
						{
							char frameText[8]{};
							SDL_snprintf(frameText, sizeof(frameText) - 1, "%d", i);

							const ImVec2 frameTextPos{ lineStart.x, frameNumbersRegion.Min.y };
							drawList->AddText(frameTextPos, IM_COL32_WHITE, frameText);
						}

						drawList->AddLine(lineStart, lineEnd, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLines]));
					}

					// --- Current frame line
					const ImVec2 frameMarkerPos = ImVec2{ keyframesRegion.Min.x + timelineFrame * frameLineDistance + padding.x - scroll,
						frameNumbersRegion.GetTL().y + menuBarHeight / 2.0f };

					const ImVec2 lineStart = frameMarkerPos;
					const ImVec2 lineEnd{ lineStart.x, keyframesRegion.Max.y };

					drawList->AddCircleFilled(frameMarkerPos, 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLinesHovered]), 4);
					drawList->AddLine(lineStart, lineEnd, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLinesHovered]));

					// --- Keyframs (finally)
					const f32 layerListStartHeight = style.FontSizeBase;

					for (auto& layer : layers)
					{
						if (layer.Expanded)
						{
							f32 xPos = keyframesRegion.Min.x + padding.x - scroll;
							f32 propStartHeight = keyframesRegion.Min.y + layerListStartHeight + style.FontSizeBase;

							DrawKeyframes<vec2>(layer.Origin, xPos, propStartHeight, drawList);
							propStartHeight += style.FontSizeBase;

							DrawKeyframes<vec2>(layer.Position, xPos, propStartHeight, drawList);
							propStartHeight += style.FontSizeBase;

							DrawKeyframes<vec2>(layer.Size, xPos, propStartHeight, drawList);
							propStartHeight += style.FontSizeBase;

							DrawKeyframes<f32>(layer.Rotation, xPos, propStartHeight, drawList);
							propStartHeight += style.FontSizeBase;

							DrawKeyframes<Color>(layer.Color, xPos, propStartHeight, drawList);
							propStartHeight += style.FontSizeBase;
						}
					}

					drawList->PopClipRect();
					ImGui::InvisibleButton("##Timeline_DopeSheet_KeyframeRegion", { frameLineDistance * animLength, 1.0f });
				}
				Gui::EndChild();
			}
			Gui::End();
		}

		bool baseMousePosNotSet{ true };
		ImVec2 baseMousePos{};
		ImVec2 mouseDrag{};

		vec2 basePoint{};
		int axisToFavor{ -1 };

		std::array<char, 64> newLayerName{};

		void RenameLayerPopUp()
		{
			if (layerToModify == nullptr || renameLayer == false)
				return;

			const ImGuiViewport* viewport = Gui::GetMainViewport();
			const ImVec2 windowPos = { viewport->Size.x / 2.0f, viewport->Size.y / 2.0f };

			Gui::SetNextWindowPos(windowPos, 0, ImVec2(0.5f, 0.5f));
			if (Gui::Begin("Rename Layer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
			{
				Gui::Text("Enter a new name for layer \"%s\"", layerToModify->Name.c_str());
				bool newNameEntered = Gui::InputText("##RenameLayer_Input", newLayerName.data(), newLayerName.size() - 1);

				if (Gui::Button("OK"))
				{
					layerToModify->Name = newLayerName.data();
					newLayerName.fill(0);
					layerToModify = nullptr;
					renameLayer = false;
				};
				Gui::SameLine();
				if (Gui::Button("Cancel"))
				{
					newLayerName.fill(0);
					layerToModify = nullptr;
					renameLayer = false;
				}
				Gui::End();
			}
		}

		const Sprite* spriteToSet{};

		void ChangeLayerSpritePopUp()
		{
			if (layerToModify == nullptr || changeLayerSprite == false)
				return;

			const ImGuiViewport* viewport = Gui::GetMainViewport();
			const ImVec2 windowPos = { viewport->Size.x / 2.0f, viewport->Size.y / 2.0f };

			Gui::SetNextWindowPos(windowPos, 0, ImVec2(0.5f, 0.5f));
			if (Gui::Begin("Change Layer Sprite", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
			{
				Gui::Text("Select a new sprite for layer \"%s\"", layerToModify->Name.c_str());

				const f32 listHeight = Gui::GetTextLineHeightWithSpacing() * 8.0f;
				if (Gui::BeginListBox("##ChangeLayerSprite_List", ImVec2(-FLT_MIN, listHeight)))
				{
					auto& sprites = spriteSheet.GetSprites();

					for (auto& sprite : sprites)
					{
						bool selected = spriteToSet == &sprite;
						ImGuiSelectableFlags flags = selected ? ImGuiSelectableFlags_Highlight : 0;
						if (Gui::Selectable(sprite.Name.c_str(), &selected, flags))
						{
							spriteToSet = &sprite;
						}
					}

					Gui::EndListBox();
				}

				if (Gui::Button("OK"))
				{
					layerToModify->Sprite = spriteToSet;

					layerToModify = nullptr;
					spriteToSet = nullptr;
					changeLayerSprite = false;
				};
				Gui::SameLine();
				if (Gui::Button("Cancel"))
				{
					layerToModify = nullptr;
					spriteToSet = nullptr;
					changeLayerSprite = false;
				}
				Gui::End();
			}
		}

		void DeleteLayerPopUp()
		{
			if (!layerToDeleteIsSet)
				return;

			const ImGuiViewport* viewport = Gui::GetMainViewport();
			const ImVec2 windowPos = { viewport->Size.x / 2.0f, viewport->Size.y / 2.0f };

			Gui::SetNextWindowPos(windowPos, 0, ImVec2(0.5f, 0.5f));
			if (Gui::Begin("Delete Layer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
			{
				Gui::Text("Are you sure you want to delete layer \"%s\"?", layerToDelete->Name.c_str());

				if (Gui::Button("Yes"))
				{
					if (&*layerToDelete == selectedLayer) { selectedLayer = nullptr; }
					layers.erase(layerToDelete);
					layerToDeleteIsSet = false;
				};
				Gui::SameLine();
				if (Gui::Button("No")) { layerToDeleteIsSet = false; }
				Gui::End();
			}
		}

		void UpdateViewportInput()
		{
			if (playing || Gui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || Gui::IsAnyItemHovered() || Gui::IsAnyItemFocused() || Gui::IsAnyItemActive())
			{
				return;
			}

			auto& io = Gui::GetIO();

			Layer* hoveredLayer = nullptr;
			for (auto& layer : layers)
			{
				const vec2 origin = layer.CurrentEditTransform.Origin;
				const vec2 pos = layer.CurrentEditTransform.Position;
				const vec2 size = layer.CurrentEditTransform.Size;
				const RectangleF layerRect(viewPan.x + pos.x - origin.x, viewPan.y + pos.y - origin.y, size.x, size.y);

				if (layerRect.Contains(io.MousePos.x, io.MousePos.y))
				{
					hoveredLayer = &layer;
				}
			}

			if (io.MouseClicked[0])
			{
				selectedLayer = hoveredLayer;
			}

			if (io.MouseDown[0] && selectedLayer != nullptr)
			{
				if (baseMousePosNotSet)
				{
					baseMousePosNotSet = false;
					baseMousePos = io.MousePos;

					basePoint = selectedLayer->CurrentEditTransform.Position;
				}

				mouseDrag.x = io.MousePos.x - baseMousePos.x;
				mouseDrag.y = io.MousePos.y - baseMousePos.y;

				if (io.KeyShift)
				{
					i32 absX = SDL_abs(mouseDrag.x);
					i32 absY = SDL_abs(mouseDrag.y);

					if (axisToFavor == -1)
					{
						if (absX > 15)
							axisToFavor = 0;
						else if (absY > 15)
							axisToFavor = 1;
					}
				}
				else
				{
					axisToFavor = -1;
				}

				vec2 newPos{};
				if (axisToFavor == 0)
				{
					newPos.x = basePoint.x + mouseDrag.x;
					newPos.y = basePoint.y;
				}
				else if (axisToFavor == 1)
				{
					newPos.x = basePoint.x;
					newPos.y = basePoint.y + mouseDrag.y;
				}
				else
				{
					newPos.x = basePoint.x + mouseDrag.x;
					newPos.y = basePoint.y + mouseDrag.y;
				}
				selectedLayer->CurrentEditTransform.Position = newPos;
			}

			if (io.MouseReleased[0])
			{
				mouseDrag = {};
				baseMousePos = {};

				basePoint = {};
				axisToFavor = -1;

				baseMousePosNotSet = true;
			}
		}

		void MainMenu()
		{
			if (Gui::BeginMainMenuBar())
			{
				if (Gui::BeginMenu("File"))
				{
					Gui::MenuItem("New");
					Gui::MenuItem("Open");
					Gui::MenuItem("Save");
					Gui::MenuItem("Save as...");

					Gui::Separator();

					Gui::MenuItem("Close");
					Gui::MenuItem("Exit");

					Gui::EndMenu();
				}
				if (Gui::BeginMenu("Debug"))
				{
					Gui::MenuItem("Metrics Window", nullptr, &showMetricsWindow);
					Gui::EndMenu();
				}
			}
			Gui::EndMainMenuBar();
		}
		
		void OnGUI()
		{
			DrawTimeline();
			MainMenu();

			RenameLayerPopUp();
			ChangeLayerSpritePopUp();
			DeleteLayerPopUp();

			UpdateViewportInput();

			if (showMetricsWindow)
			{
				Gui::ShowMetricsWindow(&showMetricsWindow);
			}
		}

		ivec2 baseViewPan{};

		void DrawCanvas(SpriteRenderer* sprRenderer)
		{
			auto& io = Gui::GetIO();

			if (io.MouseDown[2]) // Panning
			{
				if (baseMousePosNotSet)
				{
					baseMousePosNotSet = false;
					baseMousePos = io.MousePos;

					baseViewPan = viewPan;
				}

				mouseDrag.x = io.MousePos.x - baseMousePos.x;
				mouseDrag.y = io.MousePos.y - baseMousePos.y;

				viewPan.x = baseViewPan.x + mouseDrag.x;
				viewPan.y = baseViewPan.y + mouseDrag.y;
			}
			if (io.MouseReleased[2])
			{
				mouseDrag = {};
				baseMousePos = {};

				baseMousePosNotSet = true;
			}

			const vec2 realCanvasSize = vec2(canvasSize) * viewZoom;

			sprRenderer->SetSpritePosition(viewPan);
			sprRenderer->SetSpriteSize(realCanvasSize);
			sprRenderer->SetSpriteColor(DefaultColors::White);
			sprRenderer->PushSprite(nullptr);

			sprRenderer->PushOutlineRect(viewPan, realCanvasSize, {}, DefaultColors::Black);
		}

		vec2 GetKeyframeValue(f32 frame)
		{
			/*if (keyframes.size() == 0) { return currentValue; }
			else if (keyframes.size() == 1 || keyframes[0].Frame >= frame) { return keyframes[0].Value; }
			else if (keyframes.back().Frame <= frame) { return keyframes.back().Value; }

			u32 wholeFramePart = static_cast<u32>(std::floorf(frame));

			const Keyframe* start = &keyframes[0];
			const Keyframe* end = &keyframes[0];

			for (size_t i = 0; i < keyframes.size(); i++)
			{
				end = &keyframes[i];
				if (end->Frame >= wholeFramePart) break;

				start = end;
			}

			f32 t = MathExtensions::ConvertRange(start->Frame, end->Frame, 0.0f, 1.0f, frame);

			f32 x = MathExtensions::Lerp(start->Value.x, end->Value.x, t);
			f32 y = MathExtensions::Lerp(start->Value.y, end->Value.y, t);
			return { x, y };*/
			return {};
		}

		void Draw(Starshine::GameTime& gameTime)
		{
			if (playing)
			{
				timelineFrame += 1.0f;
				timelineFrame = std::fmodf(timelineFrame, animLength);
				
				//drawPos = GetKeyframeValue(timelineFrame);
			}
			else
			{
			}

			auto sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();
			auto font = GameContext::GetInstance()->DebugFont.get();

			DrawCanvas(sprRenderer);

			for (const auto& layer : layers)
			{
				const Transform2D& currentTransform = layer.CurrentEditTransform;

				i32 texIndex{};
				sprRenderer->SpriteSheet().SetSpriteState(spriteSheet, *layer.Sprite, {}, &texIndex);
				sprRenderer->SetSpriteOrigin(currentTransform.Origin);
				sprRenderer->SetSpritePosition(currentTransform.Position + viewPan);
				sprRenderer->SetSpriteSize(currentTransform.Size);
				sprRenderer->SetSpriteRotation(MathExtensions::ToRadians(currentTransform.Rotation));
				sprRenderer->SetSpriteColor(currentTransform.Color);
				sprRenderer->PushSprite(spriteSheet.GetTexture(texIndex));
			}

			if (selectedLayer != nullptr)
			{
				const Transform2D& transform = selectedLayer->CurrentEditTransform;
				static constexpr Color boxColor(255, 0, 0, 92);
				static constexpr Color borderColor(255, 0, 0, 255);

				// Bounding box
				sprRenderer->SetSpriteOrigin(transform.Origin);
				sprRenderer->SetSpritePosition(transform.Position + viewPan);
				sprRenderer->SetSpriteSize(transform.Size);
				sprRenderer->SetSpriteColor(boxColor);
				sprRenderer->PushSprite(nullptr);

				sprRenderer->PushOutlineRect(transform.Position + viewPan, transform.Size, transform.Origin, borderColor);

				// Origin axes
				sprRenderer->SetSpritePosition(vec2{ transform.Position.x, transform.Position.y - transform.Origin.y} + viewPan); // X axis
				sprRenderer->SetSpriteSize(vec2{ 1.0f, transform.Size.y });
				sprRenderer->SetSpriteColor(DefaultColors::Red);
				sprRenderer->PushSprite(nullptr);

				sprRenderer->SetSpritePosition(vec2{ transform.Position.x - transform.Origin.x, transform.Position.y } + viewPan); // Y axis
				sprRenderer->SetSpriteSize(vec2{ transform.Size.x, 1.0f });
				sprRenderer->SetSpriteColor(DefaultColors::Red);
				sprRenderer->PushSprite(nullptr);

				if (!baseMousePosNotSet) // Dragging position text
				{
					char posText[64]{};
					SDL_snprintf(posText, sizeof(posText) - 1, "X: %+.1f\nY: %+.1f", mouseDrag.x, mouseDrag.y);
					sprRenderer->Font().PushString(font, posText, vec2(mouseDrag.x + baseMousePos.x, mouseDrag.y + baseMousePos.y), vec2(1.0f), DefaultColors::White);
				}
			}

			sprRenderer->RenderSprites(nullptr);

			OnGUI();
		}
	};

	AnimEditor::AnimEditor() : impl(std::make_unique<Impl>())
	{
	}

	AnimEditor::~AnimEditor()
	{
	}

	bool AnimEditor::Initialize()
	{
		return impl->Initialize();
	}

	bool AnimEditor::LoadContent()
	{
		return impl->LoadContent();
	}

	void AnimEditor::UnloadContent()
	{
	}
	
	void AnimEditor::Destroy()
	{
	}

	void AnimEditor::Update(Starshine::GameTime& gameTime)
	{
	}

	void AnimEditor::Draw(Starshine::GameTime& gameTime)
	{
		Rendering::Device* device = Rendering::GetDevice();

		static constexpr Color clearColor{ 128, 128, 128, 255 };
		device->Clear(Rendering::ClearFlags_Color, clearColor, 1.0f, 0);

		impl->Draw(gameTime);
	}

	i64 AnimEditor::GetStateID() const
	{
		return GameState_Main;
	}
}
