#include "AnimEditor.h"
#include "Definitions.h"
#include "EditorContext.h"
#include <Rendering/Device.h>
#include <Graphics/SpriteSheet.h>
#include <Graphics/AnimationSet.h>
#include <GameContext.h>
#include <Common/MathExt.h>

#include "ResourcesWindow.h"
#include "SpriteEditorWindow.h"
#include "LayerModalWindow.h"
#include "DebugKeyframeList.h"

#include <ImGui/Core/imgui.h>
#include <ImGui/Core/imgui_internal.h>

#include "FileDialog.h"
#include <IO/Path/Path.h>
#include <IO/Path/Directory.h>
#include <Common/Logging/Logging.h>
#include <algorithm>

namespace Gui = ImGui;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine::Graphics;
using namespace Starshine;

namespace Starshine
{
	namespace ValueRanges
	{
		static constexpr vec2 MinResolution{ 480, 270 };
		static constexpr vec2 MaxResolution{ 1920, 1080 };
	}

	ImRect GetRelativeContentRegion()
	{
		const ImVec2 cursorPos = Gui::GetCursorScreenPos();
		const ImVec2 contentRegion = Gui::GetContentRegionAvail();

		return ImRect(cursorPos.x, cursorPos.y, cursorPos.x + contentRegion.x, cursorPos.y + contentRegion.y);
	}

	struct AnimEditor::Impl
	{
		AnimEditor* parent{};

		std::shared_ptr<SpriteSheet> spriteSheet{};
		EditorContextData context;

		Color stageColor{ DefaultColors::White };

		vec2 viewPan{};
		f32 viewZoom{ 1.0f };

		bool playing = false;
		bool showMetricsWindow = false;
		bool showIDStackWindow = false;

		ResourcesWindow resourcesWindow;
		SpriteEditorWindow spriteEditorWindow;
		LayerModalWindow layerModalWindow;
		DebugKeyframeList debugKeyframeListWindow;

		Impl(AnimEditor* parent) : parent(parent), resourcesWindow(&context), layerModalWindow(&context)
		{
		}

		~Impl()
		{
		}

		bool Initialize()
		{
			const RectangleF viewportSize = Rendering::GetDevice()->GetViewportSize();
			AnimationSet& animSet = context.AnimSet;

			animSet.SetResolution(vec2(1280, 720));
			animSet.SetFPS(60);
			viewPan.x = viewportSize.Width / 2.0f - 640.0f;
			viewPan.y = viewportSize.Height / 2.0f - 360.0f;

			context.CurrentAnimation = &animSet.NewAnimation("Animation 0", 0, 60);

			debugKeyframeListWindow.SetAnimation(&animSet, context.CurrentAnimation);

			return true;
		}

		bool LoadContent()
		{
			return true;
		}

		void ResetDragState()
		{
			parent->DragState.AxisToFavor = DragAxis::None;
			parent->DragState.HeldMouseButtonsMask = 0;
			parent->DragState.UserBaseValues.BaseValuesSet = false;
		}

		template <typename T>
		void InsertKeyframe(std::vector<Keyframe<T>>& keyframes, const u32& frame, const T& value)
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
				if (kf.Frame == frame)
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
			Animation* currentAnim = context.CurrentAnimation;
			auto& layers = context.Layers;

			Gui::Text("Frame");
			Gui::SameLine();
			Gui::SetNextItemWidth(56.0f);
			Gui::DragFloat("##Timeline_FrameDrag", &context.TimelineFrame, 1.0f, currentAnim->StartTime, currentAnim->EndTime, "%.0f", playing ? ImGuiSliderFlags_ReadOnly : 0);
			if (Gui::IsItemEdited() && !playing)
			{
				for (auto& layer : layers)
					layer.CurrentTransform = layer.BaseLayer->GetTransform(context.TimelineFrame);
			}

			Gui::SameLine();
			if (Gui::Button(playing ? "Stop" : "Play"))
			{
				playing = !playing;
				if (!playing)
				{
					for (auto& layer : layers)
						layer.CurrentTransform = layer.BaseLayer->GetTransform(context.TimelineFrame);
				}
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
			InsertKeyframe(keyframes, static_cast<u32>(context.TimelineFrame), value);
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

			vec4 colorTemp = value.ToVector4();
			Gui::ColorEdit4("", &colorTemp.r, ImGuiColorEditFlags_NoInputs);
			value = Color(colorTemp);

			DrawLayerProperty_End(layerIndex, keyframes, value);
			Gui::PopID();
		}

		void LayerContextMenu(size_t layerIndex)
		{
			auto editLayer = context.Layers.begin() + layerIndex;
			auto realLayers = context.CurrentAnimation->Layers;

			if (Gui::BeginPopupContextItem())
			{
				if (Gui::Selectable("Rename"))
				{
					layerModalWindow.OpenRename(layerIndex);
				}
				if (Gui::Selectable("Change sprite"))
				{
					layerModalWindow.OpenChangeSprite(layerIndex);
				}

				Gui::Separator();

				if (Gui::BeginMenu("Change order"))
				{
					ImGuiSelectableFlags selectableFlags = (layerIndex == context.Layers.size() - 1) ? ImGuiSelectableFlags_Disabled : 0;
					if (Gui::Selectable("Move to the front", false, selectableFlags))
					{
						auto layer1 = realLayers.begin() + layerIndex;
						auto layer2 = layer1 + 1;
						std::iter_swap(layer1, layer2);

						auto editLayer1 = context.Layers.begin() + layerIndex;
						auto editLayer2 = editLayer1 + 1;
						std::iter_swap(editLayer1, editLayer2);

						if (context.CurrentLayer == &*editLayer1)
							context.CurrentLayer = &*editLayer2;
					}

					selectableFlags = (layerIndex == 0) ? ImGuiSelectableFlags_Disabled : 0;
					if (Gui::Selectable("Move to the back", false, selectableFlags))
					{
						auto layer1 = realLayers.begin() + layerIndex;
						auto layer2 = layer1 - 1;
						std::iter_swap(layer1, layer2);

						auto editLayer1 = context.Layers.begin() + layerIndex;
						auto editLayer2 = editLayer1 - 1;
						std::iter_swap(editLayer1, editLayer2);

						if (context.CurrentLayer == &*editLayer1)
							context.CurrentLayer = &*editLayer2;
					}
					Gui::EndMenu();
				}

				Gui::Separator();

				if (Gui::Selectable("Position at center"))
				{
					const ivec2 stageSize = context.AnimSet.GetResolution();
					editLayer->CurrentTransform.Position.x = stageSize.x / 2.0f;
					editLayer->CurrentTransform.Position.y = stageSize.y / 2.0f;
				}

				if (Gui::BeginMenu("Set layer's origin..."))
				{
					const SpriteDefinition* spriteDef = editLayer->BaseLayer->SpriteDefinition;
					if (Gui::Selectable("at sprite's origin"))
					{
						if (spriteDef->RealSprite != nullptr)
						{
							const vec2 size = spriteDef->RealSprite->SourceRectangle.Size();
							const vec2 origin = spriteDef->RealSprite->Origin;
							editLayer->CurrentTransform.Origin = origin / size;
						}
					}
					if (Gui::Selectable("at sprite's center"))
					{
						editLayer->CurrentTransform.Origin = spriteDef->Size / 2.0f;
					}
					if (Gui::Selectable("at layer's center"))
					{
						editLayer->CurrentTransform.Origin = editLayer->CurrentTransform.Scale / 2.0f;
					}
					Gui::EndMenu();
				}

				Gui::Separator();

				if (Gui::Selectable("Remove"))
				{
					layerModalWindow.OpenRemoveLayer(layerIndex);
				}

				Gui::EndPopup();
			}
		}

		f32 layerListScroll{};

		void LayerTreeNodeProperties(i32 layerIndex, EditorLayer& layer)
		{
			LayerContextMenu(layerIndex);

			const char* blendModeName_layer = BlendModeNames[static_cast<size_t>(layer.BaseLayer->BlendMode)].data();

			Gui::SameLine();
			if (Gui::BeginCombo("##Layer_BlendMode", blendModeName_layer))
			{
				for (size_t i = 0; i < EnumCount<BlendMode>(); i++)
				{
					const char* blendModeName = BlendModeNames[i].data();
					if (Gui::Selectable(blendModeName))
						layer.BaseLayer->BlendMode = static_cast<BlendMode>(i);
				}

				Gui::EndCombo();
			}
		}

		void LayerList()
		{
			const ImRect dopeSheetRegion = GetRelativeContentRegion();

			ImVec2 layersRegionSize = dopeSheetRegion.GetBR();
			layersRegionSize.x *= 0.2f;
			layersRegionSize.y = Gui::GetContentRegionAvail().y;

			if (Gui::BeginChild("##Timeline_Layers", layersRegionSize,
				ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
			{
				auto& layers = context.Layers;

				i32 layerIndex = layers.size() - 1;
				for (auto layer = layers.rbegin(); layer != layers.rend(); layer++)
				{
					Layer* baseLayer = layer->GetLayerPointer();

					Gui::PushID(layerIndex);
					ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

					if (context.CurrentLayer == &*layer)
						nodeFlags |= ImGuiTreeNodeFlags_Selected;

					Gui::SetNextItemAllowOverlap();
					if (Gui::TreeNodeEx(baseLayer->Name.c_str(), nodeFlags))
					{
						layer->Expanded = true;
						LayerTreeNodeProperties(layerIndex, *layer);

						LayerPropertyField(layerIndex, 0, "Origin", layer->CurrentTransform.Origin, baseLayer->Origin);
						LayerPropertyField(layerIndex, 1, "Position", layer->CurrentTransform.Position, baseLayer->Position);
						LayerPropertyField(layerIndex, 2, "Scale", layer->CurrentTransform.Scale, baseLayer->Scale);
						LayerPropertyField(layerIndex, 3, "Rotation", layer->CurrentTransform.Rotation, baseLayer->Rotation);
						LayerPropertyField(layerIndex, 4, "Color", layer->CurrentTransform.Color, baseLayer->Color);

						Gui::TreePop();
					}
					else
						layer->Expanded = false;

					if (Gui::IsItemClicked())
						context.CurrentLayer = &*layer;

					if (!layer->Expanded)
						LayerTreeNodeProperties(layerIndex, *layer);

					Gui::PopID();
					layerIndex--;
				}

				layerListScroll = Gui::GetScrollY();
			}
			Gui::EndChild();
		}

		template <typename T>
		void SortKeyframes(std::vector<Keyframe<T>>& keyframes)
		{
			if (keyframes.size() <= 1)
				return;

			std::sort(keyframes.begin(), keyframes.end(),
				[](const Keyframe<T>& a, const Keyframe<T>& b) { return a.Frame < b.Frame; });
		}

		static constexpr f32 frameLineDistance = 20.0f;

		void TimelineKeyframeContextMenu(const i32& index, i32& removeIndex)
		{
			if (Gui::BeginPopupContextItem())
			{
				if (Gui::Selectable("Delete"))
				{
					removeIndex = index;
				}
				Gui::EndPopup();
			}
		}

		bool TimelineKeyframe(const i32& index, const u32& time, const vec2& position)
		{
			static constexpr f32 keyframeSize = 6.0f;
			static constexpr f32 keyframeOrigin = keyframeSize / 2.0f;

			ImDrawList* drawList = Gui::GetWindowDrawList();
			const ImGuiStyle& style = Gui::GetStyle();

			const f32 doublePadding_y = style.FramePadding.y * 2.0f;
			const f32 propCenter_y = style.FontSizeBase / 2.0f + doublePadding_y;

			const f32 keyframePosX = static_cast<f32>(time) * frameLineDistance;
			const ImVec2 keyframePos(position.x + keyframePosX, position.y + propCenter_y - keyframeOrigin);

			const ImRect keyframeRect(keyframePos.x - keyframeSize, keyframePos.y - keyframeSize,
				keyframePos.x + keyframeSize, keyframePos.y + keyframeSize);

			const ImGuiID keyframeID = Gui::GetID(index);
			Gui::PushID(keyframeID);

			Gui::ItemAdd(keyframeRect, keyframeID);

			bool hovered = false;
			bool held = false;
			Gui::ButtonBehavior(keyframeRect, keyframeID, &hovered, &held);

			if (held)
			{
				Gui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}

			ImU32 keyframeColor = (hovered || held) ? Gui::GetColorU32(style.Colors[ImGuiCol_PlotHistogramHovered])
				: Gui::GetColorU32(style.Colors[ImGuiCol_PlotHistogram]);

			drawList->AddCircleFilled(keyframePos, keyframeSize, keyframeColor, 4);
			//drawList->AddRectFilled(keyframeRect.GetTL(), keyframeRect.GetBR(), IM_COL32(255, 0, 255, 255));

			Gui::PopID(); // keyframeID
			return held;
		}

		template <typename T>
		void DrawLayerKeyframes(std::vector<Keyframe<T>>& frames, const vec2& pos, const i32& layerIndex, const i32& propIndex)
		{
			const ImGuiStyle& style = Gui::GetStyle();
			DragStateData& dragState = parent->DragState;
			Animation* currentAnim = context.CurrentAnimation;

			Gui::PushID(layerIndex);
			Gui::PushID(propIndex);

			i32 keyframeIndex = 0;
			bool sortKeyframesWhenDone = false;

			i32 keyframeToRemove = -1;
			for (auto it = frames.begin(); it != frames.end(); it++)
			{
				bool dragging = TimelineKeyframe(keyframeIndex, it->Frame, pos);
				TimelineKeyframeContextMenu(keyframeIndex++, keyframeToRemove);

				ImGuiID keyframeID = Gui::GetItemID();

				i32& draggingID = dragState.UserBaseValues.Intergers[3];

				if (dragging)
				{
					i32& baseValue = dragState.UserBaseValues.Intergers[0];
					if (!dragState.UserBaseValues.BaseValuesSet)
					{
						baseValue = it->Frame;
						draggingID = keyframeID;
						dragState.UserBaseValues.BaseValuesSet = true;
					}

					i32 dragAmount = static_cast<i32>(dragState.RelativeMousePosition.x / frameLineDistance);
					it->Frame = baseValue + dragAmount;
				}
				else if (draggingID == keyframeID)
				{
					if (dragState.UserBaseValues.BaseValuesSet)
						dragState.UserBaseValues.BaseValuesSet = false;

					if (it != frames.begin() && it->Frame < (it - 1)->Frame)
						sortKeyframesWhenDone = true;

					if ((it + 1) != frames.end() && it->Frame > (it + 1)->Frame)
						sortKeyframesWhenDone = true;
				}

				it->Frame = MathExtensions::Clamp<u32>(it->Frame, currentAnim->StartTime, currentAnim->EndTime);
			}

			Gui::PopID(); // propIndex
			Gui::PopID(); // layerIndex

			if (keyframeToRemove != -1)
				frames.erase(frames.begin() + keyframeToRemove);

			if (sortKeyframesWhenDone)
				SortKeyframes(frames);
		}

		const f32 resizeGripRange = 5.0f;
		f32 TimelineRangeResizeGrips(const ImVec2& startPos, const ImVec2& endPos, const ImGuiID& hoveredRangeID, i32& hoveredGripID)
		{
			ImGuiIO& io = Gui::GetIO();
			DragStateData& dragState = parent->DragState;

			f32 dragAmount = 0.0f;

			Gui::PushID("#Resize");
			for (i32 i = 0; i < 2; i++)
			{
				const f32 resizeGripX = i == 1 ? endPos.x : startPos.x;
				const ImRect resizeGripRect = ImRect(resizeGripX - resizeGripRange, startPos.y,
					resizeGripX + resizeGripRange, endPos.y);

				const ImGuiID resizeGripID = Gui::GetID(i + 2 + hoveredRangeID);

				bool resizeGripHovered = false;
				bool resizeGripHeld = false;

				Gui::ItemAdd(resizeGripRect, resizeGripID, 0, ImGuiItemFlags_NoNav);
				Gui::ButtonBehavior(resizeGripRect, resizeGripID, &resizeGripHovered, &resizeGripHeld, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);

				if (resizeGripHovered || resizeGripHeld)
				{
					Gui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
					hoveredGripID = i;
				}

				if (resizeGripHeld)
				{
					if (dragState.HeldMouseButtonsMask & (1 << ImGuiMouseButton_Left))
					{
						dragAmount = dragState.RelativeMousePosition.x;
						break;
					}
				}
			}
			Gui::PopID(); // "#Resize"
			return dragAmount;
		}

		i32 baseDrag = -1;
		ImGuiID draggingRangeID = 0;
		void TimelineRange(std::string_view strID, u32& start, u32& end, const vec2& position, const f32& thickness)
		{
			ImDrawList* drawList = Gui::GetWindowDrawList();

			const f32 rangeRect_startX = start * frameLineDistance;
			const f32 rangeRect_endX = end * frameLineDistance;

			const ImVec2 rangeRect_Start(position.x + rangeRect_startX, position.y);
			const ImVec2 rangeRect_End(position.x + rangeRect_endX, position.y + thickness);

			const ImRect rangeRect(rangeRect_Start, rangeRect_End);
			const ImRect rangeRect_input(rangeRect_Start.x - resizeGripRange, rangeRect_Start.y,
				rangeRect_End.x + resizeGripRange, rangeRect_End.y);

			const ImGuiID rangeID = Gui::GetID("#Range");
			Gui::PushID(rangeID);

			const ImGuiID rangeNameID = Gui::GetID(strID.data());
			Gui::PushID(rangeNameID);

			Gui::ItemAdd(rangeRect_input, rangeNameID, nullptr, 0);
			bool hovered = Gui::ItemHoverable(rangeRect_input, rangeNameID, ImGuiItemFlags_AllowOverlap);

			i32 hoveredGripID = -1;
			f32 dragAmount = TimelineRangeResizeGrips(rangeRect.GetTL(), rangeRect.GetBR(), rangeNameID, hoveredGripID);

			if (draggingRangeID != 0)
			{
				if (draggingRangeID != rangeNameID)
					goto DrawRangeAndReturn;
			}
			else
			{
				draggingRangeID = rangeNameID;
			}

			if (hoveredGripID != -1)
			{
				i32 frameAmount = static_cast<i32>(dragAmount / frameLineDistance);

				if (hoveredGripID == 0 && frameAmount != 0) // Start
				{
					if (baseDrag == -1)
						baseDrag = start;

					start = baseDrag + frameAmount;
				}
				if (hoveredGripID == 1 && frameAmount != 0) // End
				{
					if (baseDrag == -1)
						baseDrag = end;

					end = baseDrag + frameAmount;
				}
			}
			else
			{
				baseDrag = -1;
				draggingRangeID = 0;
			}

		DrawRangeAndReturn:
			const ImGuiStyle& style = Gui::GetStyle();
			ImU32 rangeColor = (hovered || hoveredGripID != -1) ? Gui::GetColorU32(style.Colors[ImGuiCol_PlotHistogramHovered])
				: Gui::GetColorU32(style.Colors[ImGuiCol_PlotHistogram]);

			drawList->AddRectFilled(rangeRect_Start, rangeRect_End, rangeColor);

			Gui::PopID(); // rangeNameID
			Gui::PopID(); // "#Range"
		}

		void DrawTimeline()
		{
			char windowTitle[128]{};
			if (context.CurrentAnimation)
				SDL_snprintf(windowTitle, sizeof(windowTitle), "Timeline - \"%s\"###Timeline", context.CurrentAnimation->Name.c_str());
			else
				SDL_snprintf(windowTitle, sizeof(windowTitle), "Timeline - No Animation###Timeline");

			Animation* currentAnim = context.CurrentAnimation;

			if (Gui::Begin(windowTitle))
			{
				ImGuiIO& io = Gui::GetIO();
				const ImGuiStyle& style = Gui::GetStyle();

				const ImVec2 contentRegion = Gui::GetContentRegionAvail();

				// NOTE: https://github.com/ocornut/imgui/issues/3284#issuecomment-641397151
				const ImVec2 padding = style.FramePadding;
				const ImVec2 itemSpacing = style.ItemSpacing;
				const f32 horizontalPadding = padding.x * 2.0f;
				const f32 verticalPadding = padding.y * 2.0f;
				const f32 verticalSpacing = itemSpacing.y * 2.0f;
				const f32 menuBarHeight = style.FontSizeBase + verticalPadding;
				const ImVec2 scrollbarHeight = { 0.0f, style.ScrollbarSize };

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
					const i32 animStart = currentAnim->StartTime;
					const i32 animLength = currentAnim->EndTime - currentAnim->StartTime;

					const ImRect frameNumbersRegion = ImRect{ timelineRegion.Min.x, timelineRegion.Min.y, timelineRegion.Max.x, timelineRegion.Min.y + menuBarHeight };
					const ImRect keyframesRegion = ImRect{ timelineRegion.Min.x, timelineRegion.Min.y + menuBarHeight, timelineRegion.Max.x,
						timelineRegion.Max.y };

					// --- Frame lines + numbers (top bar)
					drawList->AddRectFilled(keyframesRegion.GetTL(), keyframesRegion.GetBR(), Gui::GetColorU32(style.Colors[ImGuiCol_FrameBg]));
					drawList->AddRectFilled(frameNumbersRegion.GetTL(), frameNumbersRegion.GetBR(), Gui::GetColorU32(style.Colors[ImGuiCol_Button]));
					drawList->AddRect(timelineRegion.GetTL(), timelineRegion.GetBR(), Gui::GetColorU32(style.Colors[ImGuiCol_Border]));

					for (i32 i = 0; i <= animLength; i++)
					{
						const ImVec2 lineStart{ frameNumbersRegion.Min.x + i * frameLineDistance + horizontalPadding - timelineScroll, frameNumbersRegion.Max.y };
						const ImVec2 lineEnd{ lineStart.x, keyframesRegion.Max.y };

						if (i % 5 == 0)
						{
							char frameText[8]{};
							SDL_snprintf(frameText, sizeof(frameText) - 1, "%d", i + currentAnim->StartTime);

							const ImVec2 frameTextPos{ lineStart.x, frameNumbersRegion.Min.y };
							drawList->AddText(frameTextPos, IM_COL32_WHITE, frameText);
						}

						drawList->AddLine(lineStart, lineEnd, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLines]));
					}

					// --- Keyframs (finally)
					f32 layerHeight = keyframesRegion.Min.y + verticalSpacing - layerListScroll;

					drawList->PushClipRect(keyframesRegion.GetTL(), keyframesRegion.GetBR());

					vec2 keyframePos = vec2(keyframesRegion.Min.x + horizontalPadding - timelineScroll, layerHeight);

					const f32 rangeSize = style.FontSizeBase + verticalPadding;
					const f32 heightDelta = rangeSize + itemSpacing.y;

					i32 layerIndex = 0;
					auto& layers = context.Layers;
					for (auto layer = layers.rbegin(); layer != layers.rend(); layer++)
					{
						const vec2 rangePos = keyframePos;
						Layer* baseLayer = layer->GetLayerPointer();

						u32 layerStart = baseLayer->StartTime - animStart;
						u32 layerEnd = baseLayer->EndTime - animStart;
						TimelineRange(baseLayer->Name.c_str(), layerStart, layerEnd, rangePos, rangeSize);
						baseLayer->StartTime = layerStart + animStart;
						baseLayer->EndTime = layerEnd + animStart;

						baseLayer->StartTime = MathExtensions::Clamp<i32>(baseLayer->StartTime, currentAnim->StartTime, currentAnim->EndTime - 1);
						baseLayer->EndTime = MathExtensions::Clamp<i32>(baseLayer->EndTime, baseLayer->StartTime + 1, currentAnim->EndTime);

						if (layer->Expanded)
						{
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<vec2>(baseLayer->Origin, keyframePos, 0, layerIndex);
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<vec2>(baseLayer->Position, keyframePos, 1, layerIndex);
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<vec2>(baseLayer->Scale, keyframePos, 2, layerIndex);
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<f32>(baseLayer->Rotation, keyframePos, 3, layerIndex);
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<Color>(baseLayer->Color, keyframePos, 4, layerIndex);
							keyframePos.y += heightDelta;

						}
						keyframePos.y += heightDelta;

						layerIndex++;
						keyframePos.x = keyframesRegion.Min.x + horizontalPadding - timelineScroll;
					}

					drawList->PopClipRect();
					drawList->PopClipRect();
					ImGui::InvisibleButton("##Timeline_DopeSheet_KeyframeRegion", { frameLineDistance * animLength, 1.0f });

					// --- Current frame line
					const ImVec2 frameMarkerPos = ImVec2{ keyframesRegion.Min.x + context.TimelineFrame * frameLineDistance + horizontalPadding - timelineScroll,
						frameNumbersRegion.GetTL().y + menuBarHeight / 2.0f };

					const ImVec2 lineStart = frameMarkerPos;
					const ImVec2 lineEnd{ lineStart.x, keyframesRegion.Max.y };

					drawList->AddCircleFilled(frameMarkerPos, 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLinesHovered]), 4);
					drawList->AddLine(lineStart, lineEnd, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLinesHovered]));
				}
				Gui::EndChild();
			}
			Gui::End();
		}

		void UpdateDragState()
		{
			ImGuiIO& io = Gui::GetIO();
			DragStateData& dragState = parent->DragState;

			dragState.HeldMouseButtonsMask = 0;

			for (i32 i = 0; i < ImGuiMouseButton_COUNT; i++)
			{
				if (io.MouseDown[i])
					dragState.HeldMouseButtonsMask |= (1 << i);
			}

			if (dragState.HeldMouseButtonsMask != 0)
			{
				if (!dragState.BaseMousePositionSet)
				{
					dragState.BaseMousePosition = vec2(io.MousePos.x, io.MousePos.y);
					dragState.BaseMousePositionSet = true;
				}

				dragState.AbsoluteMousePosition = vec2(io.MousePos.x, io.MousePos.y);
				dragState.RelativeMousePosition = dragState.AbsoluteMousePosition - dragState.BaseMousePosition;
				dragState.DeltaMousePosition = vec2(io.MouseDelta.x, io.MouseDelta.y);

				if (dragState.AxisToFavor == DragAxis::None && io.KeyShift)
				{
					dragState.FavorOneAxis = true;

					const f32 absX = SDL_fabsf(dragState.RelativeMousePosition.x);
					const f32 absY = SDL_fabsf(dragState.RelativeMousePosition.y);

					if (absX > AxisToFavorThreshold && absX > absY)
						dragState.AxisToFavor = DragAxis::Horizontal;
					else if (absY > AxisToFavorThreshold && absY > absX)
						dragState.AxisToFavor = DragAxis::Vertical;
				}
				else
				{
					dragState.AxisToFavor = DragAxis::None;
					dragState.FavorOneAxis = false;
				}
			}
			else
			{
				if (dragState.BaseMousePositionSet)
				{
					dragState.BaseMousePosition = {};
					dragState.AbsoluteMousePosition = {};
					dragState.RelativeMousePosition = {};
					dragState.BaseMousePositionSet = false;
				}

				dragState.AxisToFavor = DragAxis::None;
				dragState.FavorOneAxis = false;
			}
		}

		void Update()
		{
			UpdateDragState();
		}

		void UpdateViewportInput()
		{
			if (playing || Gui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || Gui::IsAnyItemHovered() || Gui::IsAnyItemFocused() || Gui::IsAnyItemActive())
			{
				return;
			}

			auto& io = Gui::GetIO();
			DragStateData& dragState = parent->DragState;

			EditorLayer* hoveredLayer = nullptr;
			auto& layers = context.Layers;
			for (auto& layer : layers)
			{
				const Layer* baseLayer = layer.GetLayerPointer();

				const vec2 pos = layer.CurrentTransform.Position;
				const vec2 size = layer.CurrentTransform.Scale * baseLayer->SpriteDefinition->Size;
				const vec2 origin = layer.CurrentTransform.Origin * size;
				const RectangleF layerRect(viewPan.x + pos.x - origin.x, viewPan.y + pos.y - origin.y, size.x, size.y);

				if (layerRect.Contains(io.MousePos.x, io.MousePos.y))
					hoveredLayer = &layer;
			}

			if (io.MouseClicked[ImGuiMouseButton_Left])
				context.CurrentLayer = hoveredLayer;

			if ((dragState.HeldMouseButtonsMask & (1 << ImGuiMouseButton_Left)) && context.CurrentLayer != nullptr)
			{
				f32& basePosX = dragState.UserBaseValues.Floats[0];
				f32& basePosY = dragState.UserBaseValues.Floats[1];

				if (!dragState.UserBaseValues.BaseValuesSet)
				{
					basePosX = context.CurrentLayer->CurrentTransform.Position.x;
					basePosY = context.CurrentLayer->CurrentTransform.Position.y;
					dragState.UserBaseValues.BaseValuesSet = true;
				}

				if (dragState.FavorOneAxis)
				{
					switch (dragState.AxisToFavor)
					{
					case DragAxis::Horizontal:
						context.CurrentLayer->CurrentTransform.Position.x = basePosX + dragState.RelativeMousePosition.x;
						break;
					case DragAxis::Vertical:
						context.CurrentLayer->CurrentTransform.Position.y = basePosY + dragState.RelativeMousePosition.y;
						break;
					}
				}
				else
				{
					const vec2 basePos(basePosX, basePosY);
					context.CurrentLayer->CurrentTransform.Position = basePos + dragState.RelativeMousePosition;
				}
			}
			else
			{
				if (dragState.UserBaseValues.BaseValuesSet)
					dragState.UserBaseValues.BaseValuesSet = false;
			}
		}

		void SaveFileDialog()
		{
			FileDialog saveFileDialog;
			saveFileDialog.Title = "Save as";

			if (!saveFileDialog.OpenSave())
				return;

			context.AnimSet.WriteXml(saveFileDialog.OutputFilePath);
		}

		void ImportSpritesFromFolder(std::string_view path)
		{
			if (!IO::Directory::Exists(path))
				return;

			spriteSheet = std::make_shared<SpriteSheet>();

			SpritePacker sprPacker;
			sprPacker.AddFromDirectory(path);
			sprPacker.Pack();

			spriteSheet->CreateFromSpritePacker(sprPacker);

			auto& sprDefs = context.AnimSet.GetSpriteDefinitions();
			sprDefs.clear();

			context.AnimSet.LinkToSpriteSheet(spriteSheet);

			for (const auto& spr : spriteSheet->GetSprites())
			{
				SpriteDefinition& sprDef = sprDefs.emplace_back();
				sprDef.Name = spr.Name;
				sprDef.Size = spr.SourceRectangle.Size();
				sprDef.RealSprite = &spr;
			}

			for (auto& anim : context.AnimSet.GetAnimations())
			{
				for (auto& layer : anim.Layers)
					layer.SpriteDefinition = &context.AnimSet.GetSpriteDefinition(layer.ReferenceName);
			}

			context.AnimSet.SetSpriteSheetPath(path);

			spriteEditorWindow.SetSpriteSheet(spriteSheet.get());
		}

		void ImportSpritesFromFolderDialog()
		{
			FileDialog dialog;
			dialog.Title = "Import sprites from folder";

			if (!dialog.OpenDirectory())
				return;

			std::string path = IO::Path::GetNormalizedPath(dialog.OutputFilePath);

			ImportSpritesFromFolder(path);
		}

		void OpenFileDialog()
		{
			FileDialog openFileDialog;
			openFileDialog.Title = "Open";

			if (!openFileDialog.OpenRead())
				return;

			context.CurrentAnimation = nullptr;
			context.AnimSet.GetSpriteDefinitions().clear();
			context.AnimSet.GetAnimations().clear();

			context.AnimSet.LoadXml(openFileDialog.OutputFilePath);
			ImportSpritesFromFolder(context.AnimSet.GetSpriteSheetPath());

			context.CurrentAnimation = &context.AnimSet.GetAnimations().front();
			context.RecreateLayerList();

			debugKeyframeListWindow.SetAnimation(&context.AnimSet, context.CurrentAnimation);
		}

		void MainMenu()
		{
			if (Gui::BeginMainMenuBar())
			{
				if (Gui::BeginMenu("File"))
				{
					/*if (Gui::MenuItem("New"))
					{
						currentAnim->Layers.clear();
						//selectedLayer = nullptr;
						timelineFrame = 0.0f;
					}*/
					if (Gui::MenuItem("Open"))
						OpenFileDialog();
					if (Gui::MenuItem("Save"))
						SaveFileDialog();
					if (Gui::MenuItem("Save as..."))
						SaveFileDialog();

					Gui::Separator();

					if (Gui::BeginMenu("Import sprites..."))
					{
						if (Gui::MenuItem("From folder"))
							ImportSpritesFromFolderDialog();

						Gui::MenuItem("From file");
						Gui::EndMenu();
					}

					Gui::Separator();

					Gui::MenuItem("Exit");

					Gui::EndMenu();
				}

				if (Gui::BeginMenu("Edit"))
				{
					if (Gui::MenuItem("Animation Set Properties"))
						animSetProperites_display = true;

					if (Gui::MenuItem("Sprite Editor"))
						spriteEditorWindow.DrawWindow = true;

					Gui::EndMenu();
				}

				if (Gui::BeginMenu("Debug"))
				{
					Gui::MenuItem("Metrics Window", nullptr, &showMetricsWindow);
					Gui::MenuItem("ID Stack Window", nullptr, &showIDStackWindow);
					Gui::MenuItem("Keyframe List Window", nullptr, &debugKeyframeListWindow.DrawWindow);
					Gui::EndMenu();
				}
			}
			Gui::EndMainMenuBar();
		}

		constexpr f32 GetCubicBezierPoint(const f32& start, const f32& end, const f32& t)
		{
			const f32 tSquared = t * t;
			const f32 tCubed = t * t * t;
			const f32 tInv = 1.0f - t;

			f32 value = 3 * (tInv * tInv) * t * start;
			value += 3 * tInv * tSquared * end;
			value += tCubed * 1.0f;

			return value;
		}

		vec2 easingTime{ 0.5f, 0.5f };
		vec2 easingValue{ 0.5f, 0.5f };

		void EasingPlotWindow()
		{
			f32 hermitePoints[60]{};
			f32 t = 0.0f;
			for (size_t i = 0.0f; i < 60; i++)
			{
				t += 1.0f / 60.0f;
				f32 easedT = GetCubicBezierPoint(easingTime.x, easingTime.y, t);
				hermitePoints[i] = GetCubicBezierPoint(easingValue.x, easingValue.y, easedT);
			}

			if (Gui::Begin("Easing"))
			{
				const ImVec2 contentRegion = Gui::GetContentRegionAvail();

				Gui::Text("Graph");

				Gui::PlotLines("##PlotPoints", hermitePoints, 60, 0, nullptr, 0.0f, 1.0f, ImVec2(contentRegion.x, contentRegion.y - 128.0f));

				Gui::Text("Time");
				Gui::SameLine();
				Gui::SetNextItemWidth(128.0f);
				Gui::DragFloat2("##EaseTime", &easingTime.x, 0.1f, 0.0f, 1.0f);

				Gui::Text("Value");
				Gui::SameLine();
				Gui::SetNextItemWidth(128.0f);
				Gui::DragFloat2("##EaseValue", &easingValue.x, 0.1f, 0.0f, 1.0f);

				Gui::End();
			}
		}

		bool animSetProperites_display = false;
		void AnimationSetPropertiesWindow()
		{
			if (!animSetProperites_display)
				return;

			if (Gui::Begin("Animation Set Properties", &animSetProperites_display))
			{
				Gui::Text("Resolution");
				Gui::SameLine();
				
				ivec2 animRes = context.AnimSet.GetResolution();
				i32 animFPS = context.AnimSet.GetFPS();

				Gui::DragInt2("##AnimSetProperties_Resolution", &animRes[0]);
				if (Gui::IsItemEdited())
				{
					animRes.x = MathExtensions::Clamp<i32>(animRes.x, ValueRanges::MinResolution.x, ValueRanges::MaxResolution.x);
					animRes.y = MathExtensions::Clamp<i32>(animRes.y, ValueRanges::MinResolution.y, ValueRanges::MaxResolution.y);
					context.AnimSet.SetResolution(animRes);
				}

				Gui::Text("FPS");
				Gui::SameLine();
				Gui::DragInt("##AnimSetProperties_FPS", &animFPS, 1.0f, 30, 120);
				if (Gui::IsItemEdited())
					context.AnimSet.SetFPS(animFPS);

				Gui::Text("Stage Color");
				Gui::SameLine();

				vec4 stageColor_vec4 = stageColor.ToVector4();
				Gui::ColorEdit3("##AnimSetProperties_StageColor", &stageColor_vec4[0]);
				if (Gui::IsItemEdited())
					stageColor = Color(stageColor_vec4);

				Gui::End();
			}
		}

		void OnGUI()
		{
			DrawTimeline();
			spriteEditorWindow.OnGUI();
			resourcesWindow.OnGUI();

			layerModalWindow.OnGUI();
			
			AnimationSetPropertiesWindow();

			UpdateViewportInput();

			MainMenu();

			if (showMetricsWindow)
				Gui::ShowMetricsWindow(&showMetricsWindow);
			if (showIDStackWindow)
				Gui::ShowIDStackToolWindow(&showIDStackWindow);

			debugKeyframeListWindow.OnGUI();
		}

		ivec2 baseViewPan{};

		void DrawCanvas(SpriteRenderer* sprRenderer)
		{
			DragStateData& dragState = parent->DragState;

			if (dragState.HeldMouseButtonsMask & (1 << ImGuiMouseButton_Middle)) // Panning
				viewPan += dragState.DeltaMousePosition;

			const vec2 realCanvasSize = vec2(context.AnimSet.GetResolution()) * viewZoom;

			sprRenderer->SetSpritePosition(viewPan);
			sprRenderer->SetSpriteSize(realCanvasSize);
			sprRenderer->SetSpriteColor(stageColor);
			sprRenderer->PushSprite(nullptr);

			sprRenderer->PushOutlineRect(viewPan, realCanvasSize, {}, DefaultColors::Black);
		}

		void Draw(Starshine::GameTime& gameTime)
		{
			auto sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();
			auto font = GameContext::GetInstance()->DebugFont.get();
			
			DrawCanvas(sprRenderer);

			if (playing)
			{
				sprRenderer->AnimationSet().PushAnimation(&context.AnimSet, context.CurrentAnimation, context.TimelineFrame, viewPan, vec2(viewZoom));
				sprRenderer->RenderSprites(nullptr);

				context.TimelineFrame += context.AnimSet.GetRelativeFrameTimeStep(gameTime.ElapsedFrameTime.GetSeconds());
				context.TimelineFrame = std::fmodf(context.TimelineFrame, context.CurrentAnimation->EndTime) + context.CurrentAnimation->StartTime;
			}
			else
			{
				DragStateData& dragState = parent->DragState;
				BlendMode prevBlendMode = BlendMode::Normal;

				for (const auto& editLayer : context.Layers)
				{
					const Layer* baseLayer = editLayer.GetLayerPointer();

					if (!baseLayer->Visible)
						continue;

					const Transform2D transform = editLayer.CurrentTransform;
					const SpriteDefinition* spriteDef = baseLayer->SpriteDefinition;
					const vec2& spriteSize = spriteDef->Size * viewZoom;
					const vec2& spriteLayerSize = spriteSize * transform.Scale;

					if (prevBlendMode != baseLayer->BlendMode)
					{
						vec2 basePos{};
						vec2 baseScale{};

						sprRenderer->RenderSprites(nullptr);
						sprRenderer->SetBlendMode(baseLayer->BlendMode);
					}

					i32 texIndex{};

					if (spriteDef->RealSprite != nullptr)
						sprRenderer->SpriteSheet().SetSpriteState(*spriteSheet, *spriteDef->RealSprite, vec2{}, &texIndex);

					sprRenderer->SetSpriteOrigin(transform.Origin * spriteLayerSize);
					sprRenderer->SetSpritePosition(transform.Position + viewPan);
					sprRenderer->SetSpriteSize(transform.Scale * spriteSize);
					sprRenderer->SetSpriteRotation(MathExtensions::ToRadians(transform.Rotation));
					sprRenderer->SetSpriteColor(transform.Color);

					if (spriteDef->RealSprite != nullptr)
						sprRenderer->PushSprite(spriteSheet->GetTexture(texIndex));
					else
						sprRenderer->PushSprite(nullptr);

					prevBlendMode = baseLayer->BlendMode;
				}

				sprRenderer->SetBlendMode(BlendMode::Normal);
				if (context.CurrentLayer != nullptr)
				{
					const Transform2D& transform = context.CurrentLayer->CurrentTransform;
					const SpriteDefinition* spriteDef = context.CurrentLayer->BaseLayer->SpriteDefinition;
					const vec2& spriteSize = spriteDef->Size * viewZoom;
					const vec2& spriteLayerSize = spriteSize * transform.Scale;
					const vec2& spriteLayerOrigin = spriteLayerSize * transform.Origin;

					static constexpr Color boxColor(255, 0, 0, 92);
					static constexpr Color borderColor(255, 0, 0, 255);

					// Bounding box
					sprRenderer->SetSpriteOrigin(spriteLayerOrigin);
					sprRenderer->SetSpritePosition(transform.Position + viewPan);
					sprRenderer->SetSpriteSize(spriteLayerSize);
					sprRenderer->SetSpriteColor(boxColor);
					sprRenderer->PushSprite(nullptr);

					sprRenderer->PushOutlineRect(transform.Position + viewPan, spriteLayerSize, spriteLayerOrigin, borderColor);

					// Origin axes
					sprRenderer->SetSpritePosition(vec2{ transform.Position.x, transform.Position.y - spriteLayerOrigin.y } + viewPan); // X axis
					sprRenderer->SetSpriteSize(vec2{ 1.0f, spriteLayerSize.y });
					sprRenderer->SetSpriteColor(DefaultColors::Red);
					sprRenderer->PushSprite(nullptr);

					sprRenderer->SetSpritePosition(vec2{ transform.Position.x - spriteLayerOrigin.x, transform.Position.y } + viewPan); // Y axis
					sprRenderer->SetSpriteSize(vec2{ spriteLayerSize.x, 1.0f });
					sprRenderer->SetSpriteColor(DefaultColors::Red);
					sprRenderer->PushSprite(nullptr);

					if (dragState.HeldMouseButtonsMask != 0) // Dragging position text
					{
						char posText[64]{};
						const vec2 relMousePos = dragState.RelativeMousePosition;
						const vec2 absMousePos = dragState.AbsoluteMousePosition;

						SDL_snprintf(posText, sizeof(posText) - 1, "X: %+.1f\nY: %+.1f", relMousePos.x, relMousePos.y);
						sprRenderer->Font().DrawString(font, posText, absMousePos, vec2(1.0f), DefaultColors::White);
					}

					sprRenderer->RenderSprites(nullptr);
				}
				sprRenderer->RenderSprites(nullptr);
			}
			OnGUI();
		}
	};

	AnimEditor::AnimEditor() : impl(std::make_unique<Impl>(this))
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
		impl->Update();
	}

	void AnimEditor::Draw(Starshine::GameTime& gameTime)
	{
		Rendering::Device* device = Rendering::GetDevice();

		static constexpr Color clearColor{ 128, 128, 128, 255 };
		device->Clear(Rendering::ClearFlags_Color, clearColor, 1.0f, 0);

		impl->Draw(gameTime);
	}

	void AnimEditor::ResetDragState()
	{
		impl->ResetDragState();
	}
}
