#include "AnimEditor.h"
#include "Definitions.h"
#include <Rendering/Device.h>
#include <Rendering/Render2D/SpriteSheet.h>
#include <GameContext.h>
#include <Common/MathExt.h>
#include <Input/Mouse.h>

#include <ImGui/Core/imgui.h>
#include <ImGui/Core/imgui_internal.h>

#include "FileDialog.h"
#include <IO/Xml.h>

namespace Gui = ImGui;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine;

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

		Keyframe() {};
		Keyframe(const f32& frame, const T& value) : Frame(frame), Value(value) {};
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

	template <typename T>
	bool InterpolateKeyframes(const std::vector<Keyframe<T>>& keyframes, const f32& frame, T& value)
	{
		if (keyframes.empty())
			return false;

		if (keyframes.size() == 1)
		{
			value = keyframes[0].Value;
			return true;
		}

		const Keyframe<T>& last = keyframes.back();

		if (frame >= last.Frame)
		{
			value = last.Value;
			return true;
		}

		const Keyframe<T>* start = &keyframes[0];
		const Keyframe<T>* end = start;

		for (size_t i = 0; i < keyframes.size(); i++)
		{
			end = &keyframes[i];
			if (end->Frame >= frame)
				break;

			start = end;
		}

		f32 frameFactor = MathExtensions::ConvertRange<f32>(start->Frame, end->Frame, 0.0f, 1.0f, frame);
		value = start->Value * (1.0f - frameFactor) + end->Value * frameFactor;

		return true;
	}

	Transform2D GetTransformAtFrame(const Layer& layer, f32 frame)
	{
		Transform2D result{};
		result.Origin = layer.Sprite->Origin;
		result.Size = layer.Sprite->SourceRectangle.Size();
		result.Color = DefaultColors::White;

		vec2 tempVec2{};
		Color tempColor{};

		if (InterpolateKeyframes(layer.Origin, frame, tempVec2))
			result.Origin = tempVec2;
		if (InterpolateKeyframes(layer.Position, frame, tempVec2))
			result.Position = tempVec2;
		if (InterpolateKeyframes(layer.Size, frame, tempVec2))
			result.Size = tempVec2;
		if (InterpolateKeyframes(layer.Rotation, frame, tempVec2.x))
			result.Rotation = tempVec2.x;
		if (InterpolateKeyframes(layer.Color, frame, tempColor))
			result.Color = tempColor;

		return result;
	}

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
		bool showIDStackWindow = false;

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

		template <typename T>
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

				Keyframe<T>& kf = keyframes[i];
				if ((i32)kf.Frame == (i32)frame)
				{
					kf.Value = value;
					return;
				}
				if (frame > kf.Frame)
				{
					continue;
				}

				keyframes.emplace(placement, Keyframe<T>(frame, value));
				return;
			}

			keyframes.push_back(Keyframe<T>(frame, value));
		}

		void TimelineHeader()
		{
			Gui::Text("Frame");
			Gui::SameLine();
			Gui::SetNextItemWidth(56.0f);
			Gui::DragFloat("##Timeline_FrameDrag", &timelineFrame, 1.0f, 0.0f, animLength - 1.0f, "%.0f", playing ? ImGuiSliderFlags_ReadOnly : 0);

			Gui::SameLine();
			if (Gui::Button("Play"))
			{
				playing = !playing;
			}
		}

		void DrawLayerProperty_Begin(const char* propName)
		{
			Gui::Text(propName);

			Gui::SameLine();
			Gui::SetNextItemWidth(192.0f);
		}

		template <typename T>
		void InsertKeyframeButton(std::vector<Keyframe<T>>& keyframes, const T& value)
		{
			InsertKeyframe(keyframes, timelineFrame, value);
		}

		template <typename T>
		void DrawLayerProperty_End(i32 layerIndex, std::vector<Keyframe<T>>& keyframes, const T& value)
		{
			Gui::SameLine();
			if (Gui::Button("+F"))
			{
				InsertKeyframeButton(keyframes, value);
			}
		}

		void LayerPropertyField(i32 layerIndex, i32 propIndex, std::string_view propName, vec2& value, std::vector<Keyframe<vec2>>& keyframes)
		{
			const char* propName_data = propName.data();

			DrawLayerProperty_Begin(propName_data);
			Gui::PushID(propIndex);
			Gui::DragFloat2("", &value.x);
			DrawLayerProperty_End(layerIndex, keyframes, value);
			Gui::PopID();
		}

		void LayerPropertyField(i32 layerIndex, i32 propIndex, std::string_view propName, f32& value, std::vector<Keyframe<f32>>& keyframes)
		{
			const char* propName_data = propName.data();

			DrawLayerProperty_Begin(propName_data);
			Gui::PushID(propIndex);
			Gui::DragFloat("", &value);
			DrawLayerProperty_End(layerIndex, keyframes, value);
			Gui::PopID();
		}

		void LayerPropertyField(i32 layerIndex, i32 propIndex, std::string_view propName, Color& value, std::vector<Keyframe<Color>>& keyframes)
		{
			const char* propName_data = propName.data();

			DrawLayerProperty_Begin(propName_data);
			Gui::PushID(propIndex);

			static vec4 colorTemp = value.ToVector4();
			Gui::ColorEdit4("", &colorTemp.r, ImGuiColorEditFlags_NoInputs);
			value = Color(colorTemp);

			DrawLayerProperty_End(layerIndex, keyframes, value);
			Gui::PopID();
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
				if (Gui::Selectable("Change sprite"))
				{
					layerToModify = &*layer;
					changeLayerSprite = true;
				}

				Gui::Separator();

				if (Gui::BeginMenu("Change order"))
				{
					ImGuiSelectableFlags selectableFlags = (layerIndex == 0) ? ImGuiSelectableFlags_Disabled : 0;
					if (Gui::Selectable("Move upwards", false, selectableFlags))
					{
						auto layer1 = layers.begin() + layerIndex;
						auto layer2 = layer1 - 1;
						std::iter_swap(layer1, layer2);

						if (selectedLayer == &*layer1)
						{
							selectedLayer = &*layer2;
						}
					}

					selectableFlags = (layerIndex == layers.size() - 1) ? ImGuiSelectableFlags_Disabled : 0;
					if (Gui::Selectable("Move downwards", false, selectableFlags))
					{
						auto layer1 = layers.begin() + layerIndex;
						auto layer2 = layer1 + 1;
						std::iter_swap(layer1, layer2);

						if (selectedLayer == &*layer1)
						{
							selectedLayer = &*layer2;
						}
					}
					Gui::EndMenu();
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

		f32 layerListScroll{};

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
					ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

					if (selectedLayer == &*layer)
						nodeFlags |= ImGuiTreeNodeFlags_Selected;

					if (Gui::TreeNodeEx(layer->Name.c_str(), nodeFlags))
					{
						layer->Expanded = true;

						LayerPropertyField(layerIndex, 0, "Origin", layer->CurrentEditTransform.Origin, layer->Origin);
						LayerPropertyField(layerIndex, 1, "Position", layer->CurrentEditTransform.Position, layer->Position);
						LayerPropertyField(layerIndex, 2, "Size", layer->CurrentEditTransform.Size, layer->Size);
						LayerPropertyField(layerIndex, 3, "Rotation", layer->CurrentEditTransform.Rotation, layer->Rotation);
						LayerPropertyField(layerIndex, 4, "Color", layer->CurrentEditTransform.Color, layer->Color);
						Gui::TreePop();
					}
					else
					{
						layer->Expanded = false;
					}

					if (Gui::IsItemClicked())
						selectedLayer = &*layer;

					LayerContextMenu(layerIndex);
					Gui::PopID();
					layerIndex++;
				}

				layerListScroll = Gui::GetScrollY();
			}
			Gui::EndChild();
		}

		static constexpr f32 frameLineDistance = 15.0f;

		template <typename T>
		void DrawKeyframes(const std::vector<Keyframe<T>>& frames, const f32& xPos, const f32& yPos, const f32& yPos_duplicate, bool drawDuplicate, ImDrawList* drawList)
		{
			const ImGuiStyle& style = Gui::GetStyle();

			for (auto& keyframe : frames)
			{
				const f32 framePos_x = xPos + keyframe.Frame * frameLineDistance;

				drawList->AddCircleFilled(ImVec2(framePos_x, yPos), 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_Text]), 4);
				if (drawDuplicate)
					drawList->AddCircleFilled(ImVec2(framePos_x, yPos_duplicate), 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_Text]), 4);
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
				const ImVec2 padding = style.FramePadding;
				const ImVec2 itemSpacing = style.ItemSpacing;
				const f32 verticalPadding = padding.y * 2.0f;
				const f32 verticalSpacing = itemSpacing.y * 2.0f;
				const f32 menuBarHeight = style.FontSizeBase + verticalPadding;

				TimelineHeader();
				LayerList();

				// --- Timeline (keyframes)
				ImGui::SameLine();

				const ImRect timelineRegion = GetRelativeContentRegion();
				if (Gui::BeginChild("##Timeline_DopeSheet_Keyframes", {}, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_HorizontalScrollbar))
				{
					ImDrawList* drawList = Gui::GetWindowDrawList();
					drawList->PushClipRect(timelineRegion.GetTL(), timelineRegion.GetBR());

					f32 timelineScroll = Gui::GetScrollX();

					const ImRect frameNumbersRegion = ImRect{ timelineRegion.Min.x, timelineRegion.Min.y, timelineRegion.Max.x, timelineRegion.Min.y + menuBarHeight };
					const ImRect keyframesRegion = ImRect{ timelineRegion.Min.x, timelineRegion.Min.y + menuBarHeight, timelineRegion.Max.x, timelineRegion.Max.y - style.ScrollbarSize };

					// --- Frame lines + numbers (top bar)
					drawList->AddRectFilled(keyframesRegion.GetTL(), keyframesRegion.GetBR(), Gui::GetColorU32(style.Colors[ImGuiCol_FrameBg]));
					drawList->AddRectFilled(frameNumbersRegion.GetTL(), frameNumbersRegion.GetBR(), Gui::GetColorU32(style.Colors[ImGuiCol_Button]));
					drawList->AddRect(timelineRegion.GetTL(), timelineRegion.GetBR(), Gui::GetColorU32(style.Colors[ImGuiCol_Border]));

					for (i32 i = 0; i < (i32)animLength; i++)
					{
						const ImVec2 lineStart{ frameNumbersRegion.Min.x + i * frameLineDistance + padding.x - timelineScroll, frameNumbersRegion.Max.y };
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
					const ImVec2 frameMarkerPos = ImVec2{ keyframesRegion.Min.x + timelineFrame * frameLineDistance + padding.x - timelineScroll,
						frameNumbersRegion.GetTL().y + menuBarHeight / 2.0f };

					const ImVec2 lineStart = frameMarkerPos;
					const ImVec2 lineEnd{ lineStart.x, keyframesRegion.Max.y };

					drawList->AddCircleFilled(frameMarkerPos, 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLinesHovered]), 4);
					drawList->AddLine(lineStart, lineEnd, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLinesHovered]));

					// --- Keyframs (finally)
					const f32 layerListStartHeight = style.FontSizeBase;
					f32 layerHeight = keyframesRegion.Min.y + layerListStartHeight - layerListScroll;

					drawList->PushClipRect(keyframesRegion.GetTL(), keyframesRegion.GetBR());

					for (auto& layer : layers)
					{
						f32 xPos = keyframesRegion.Min.x + padding.x - timelineScroll;
						f32 propStartHeight = layerHeight + style.FontSizeBase;

						bool drawDuplicates = layer.Expanded;

						DrawKeyframes<vec2>(layer.Origin, xPos, layerHeight, propStartHeight, drawDuplicates, drawList);
						propStartHeight += style.FontSizeBase + verticalSpacing + padding.y;

						DrawKeyframes<vec2>(layer.Position, xPos, layerHeight, propStartHeight, drawDuplicates, drawList);
						propStartHeight += style.FontSizeBase + verticalSpacing + padding.y;

						DrawKeyframes<vec2>(layer.Size, xPos, layerHeight, propStartHeight, drawDuplicates, drawList);
						propStartHeight += style.FontSizeBase + verticalSpacing + padding.y;

						DrawKeyframes<f32>(layer.Rotation, xPos, layerHeight, propStartHeight, drawDuplicates, drawList);
						propStartHeight += style.FontSizeBase + verticalSpacing + padding.y;

						DrawKeyframes<Color>(layer.Color, xPos, layerHeight, propStartHeight, drawDuplicates, drawList);
						propStartHeight += style.FontSizeBase + verticalSpacing + padding.y;

						if (drawDuplicates)
							layerHeight = propStartHeight;
						else
							layerHeight += style.FontSizeBase + padding.y;
					}

					drawList->PopClipRect();
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

		template <typename T>
		void WriteKeyframes_Xml(const std::vector<Keyframe<T>>& keyframes, std::string_view elementName, Xml::Element* layerElement)
		{
			if (keyframes.empty())
				return;

			Xml::Element* frameListElement = layerElement->InsertNewChildElement(elementName.data());
			for (const auto& frame : keyframes)
			{
				Xml::Element* frameElement = frameListElement->InsertNewChildElement("Keyframe");
				frameElement->SetAttribute("Frame", static_cast<i32>(frame.Frame));
				Xml::SetAttribute(frameElement, "Value", frame.Value);
			}
		}

		void WriteKeyframes_Xml(const std::vector<Keyframe<f32>>& keyframes, std::string_view elementName, Xml::Element* layerElement)
		{
			if (keyframes.empty())
				return;

			Xml::Element* frameListElement = layerElement->InsertNewChildElement(elementName.data());
			for (const auto& frame : keyframes)
			{
				Xml::Element* frameElement = frameListElement->InsertNewChildElement("Keyframe");
				frameElement->SetAttribute("Frame", static_cast<i32>(frame.Frame));
				frameElement->SetAttribute("Value", frame.Value);
			}
		}
		
		void SaveFileDialog()
		{
			FileDialog saveFileDialog;
			saveFileDialog.Title = "Save as";

			if (!saveFileDialog.OpenSave())
				return;

			Xml::Document animSetDoc;
			Xml::Element* rootElement = animSetDoc.NewElement("AnimationSet");

			rootElement->SetAttribute("Width", canvasSize.x);
			rootElement->SetAttribute("Height", canvasSize.y);
			rootElement->SetAttribute("FPS", 60);
			Xml::SetAttribute(rootElement, "StageColor", DefaultColors::White);
			rootElement->SetAttribute("SpriteSheet", "sprites/devtest");

			Xml::Element* animElement = rootElement->InsertNewChildElement("Animation");
			animElement->SetAttribute("Name", "Testing00");
			animElement->SetAttribute("Duration", 60u);

			for (const auto& layer : layers)
			{
				Xml::Element* layerElement = animElement->InsertNewChildElement("Layer");
				layerElement->SetAttribute("Name", layer.Name.c_str());
				layerElement->SetAttribute("Sprite", layer.Sprite->Name.c_str());
				layerElement->SetAttribute("Start", static_cast<i32>(layer.StartTime));
				layerElement->SetAttribute("Duration", static_cast<i32>(layer.Duration));

				// Oh boy, I can't wait to start abusing the C++ type system!
				WriteKeyframes_Xml(layer.Origin, "Origin", layerElement);
				WriteKeyframes_Xml(layer.Position, "Position", layerElement);
				WriteKeyframes_Xml(layer.Size, "Size", layerElement);
				WriteKeyframes_Xml(layer.Rotation, "Rotation", layerElement);
				WriteKeyframes_Xml(layer.Color, "Color", layerElement);
			}

			animSetDoc.InsertFirstChild(rootElement);
			animSetDoc.SaveFile(saveFileDialog.OutputFilePath.c_str());
		}

		void MainMenu()
		{
			if (Gui::BeginMainMenuBar())
			{
				if (Gui::BeginMenu("File"))
				{
					Gui::MenuItem("New");
					Gui::MenuItem("Open");
					if (Gui::MenuItem("Save"))
						SaveFileDialog();
					if (Gui::MenuItem("Save as..."))
						SaveFileDialog();

					Gui::Separator();

					Gui::MenuItem("Close");
					Gui::MenuItem("Exit");

					Gui::EndMenu();
				}
				if (Gui::BeginMenu("Debug"))
				{
					Gui::MenuItem("Metrics Window", nullptr, &showMetricsWindow);
					Gui::MenuItem("ID Stack Window", nullptr, &showIDStackWindow);
					Gui::EndMenu();
				}
			}
			Gui::EndMainMenuBar();
		}
		
		void OnGUI()
		{
			DrawTimeline();

			RenameLayerPopUp();
			ChangeLayerSpritePopUp();
			DeleteLayerPopUp();

			UpdateViewportInput();

			MainMenu();

			if (showMetricsWindow)
				Gui::ShowMetricsWindow(&showMetricsWindow);
			if (showIDStackWindow)
				Gui::ShowIDStackToolWindow(&showIDStackWindow);
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

		void DrawAnimation(SpriteRenderer* sprRenderer)
		{
			for (const auto& layer : layers)
			{
				const Transform2D transform = GetTransformAtFrame(layer, timelineFrame);

				i32 texIndex{};
				sprRenderer->SpriteSheet().SetSpriteState(spriteSheet, *layer.Sprite, {}, &texIndex);
				sprRenderer->SetSpriteOrigin(transform.Origin);
				sprRenderer->SetSpritePosition(transform.Position + viewPan);
				sprRenderer->SetSpriteSize(transform.Size);
				sprRenderer->SetSpriteRotation(MathExtensions::ToRadians(transform.Rotation));
				sprRenderer->SetSpriteColor(transform.Color);
				sprRenderer->PushSprite(spriteSheet.GetTexture(texIndex));
			}
		}

		void Draw(Starshine::GameTime& gameTime)
		{
			auto sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();
			auto font = GameContext::GetInstance()->DebugFont.get();
			
			DrawCanvas(sprRenderer);

			if (playing)
			{
				DrawAnimation(sprRenderer);

				timelineFrame += 1.0f;
				timelineFrame = std::fmodf(timelineFrame, animLength);
			}
			else
			{
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
					sprRenderer->SetSpritePosition(vec2{ transform.Position.x, transform.Position.y - transform.Origin.y } + viewPan); // X axis
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
