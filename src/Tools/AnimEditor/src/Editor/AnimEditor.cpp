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
#include <IO/Path/Path.h>
#include <IO/Path/Directory.h>
#include <Common/Logging/Logging.h>
#include <algorithm>

namespace Gui = ImGui;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine;

namespace Starshine
{
	namespace XmlElementNames
	{
		static constexpr const char* AnimationSet = "AnimationSet";
		static constexpr const char* AnimationSet_Width = "Width";
		static constexpr const char* AnimationSet_Height = "Height";
		static constexpr const char* AnimationSet_FPS = "FPS";
		static constexpr const char* AnimationSet_StageColor = "StageColor";
		static constexpr const char* AnimationSet_SpriteSheet = "SpriteSheet";

		static constexpr const char* Common_Name = "Name";
		static constexpr const char* Common_Start = "Start";
		static constexpr const char* Common_End = "End";

		static constexpr const char* Animation = "Animation";
		static constexpr const char* AnimationLayer = "Layer";
		static constexpr const char* AnimationLayer_Sprite = "Sprite";
		static constexpr const char* AnimationLayer_BlendMode = "BlendMode";

		static constexpr std::string_view Keyframes_Origin = "Origin";
		static constexpr std::string_view Keyframes_Position = "Position";
		static constexpr std::string_view Keyframes_Size = "Size";
		static constexpr std::string_view Keyframes_Rotation = "Rotation";
		static constexpr std::string_view Keyframes_Color = "Color";

		static constexpr const char* Keyframe = "Keyframe";
		static constexpr const char* Keyframe_Frame = "Frame";
		static constexpr const char* Keyframe_Value = "Value";
	}

	namespace ValueRanges
	{
		static constexpr vec2 MinResolution{ 480, 270 };
		static constexpr vec2 MaxResolution{ 1920, 1080 };
	}

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
		u32 Frame{};
		T Value{};

		Keyframe() {};
		Keyframe(const u32& frame, const T& value) : Frame(frame), Value(value) {};
	};

	struct Layer
	{
		std::string Name;

		u32 StartTime{};
		u32 EndTime{};

		BlendMode BlendMode{};

		std::vector<Keyframe<vec2>> Origin;
		std::vector<Keyframe<vec2>> Position;
		std::vector<Keyframe<vec2>> Size;
		std::vector<Keyframe<f32>> Rotation;
		std::vector<Keyframe<Color>> Color;
		const Sprite* Sprite{};

		Transform2D CurrentEditTransform{};
		bool Expanded{};
	};

	struct Animation
	{
		std::string Name;

		u32 StartTime{};
		u32 EndTime{};

		std::vector<Layer> Layers;
	};

	template <typename T>
	bool InterpolateKeyframes(const std::vector<Keyframe<T>>& keyframes, const f32& frame, T& value)
	{
		if (keyframes.empty())
			return false;

		if (keyframes.size() == 1 || frame <= 0.0f)
		{
			value = keyframes[0].Value;
			return true;
		}

		const Keyframe<T>& first = keyframes.front();
		const Keyframe<T>& last = keyframes.back();

		if (frame <= first.Frame)
		{
			value = first.Value;
			return true;
		}

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

		const f32 range = static_cast<f32>(end->Frame - start->Frame);
		const f32 f = frame - static_cast<f32>(start->Frame);
		const f32 frameFactor = MathExtensions::ConvertRange<f32>(0.0f, range, 0.0f, 1.0f, f);
		value = start->Value * (1.0f - frameFactor) + end->Value * frameFactor;

		return true;
	}

	Transform2D GetTransformAtFrame(const Layer& layer, const f32& frame)
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
		AnimEditor* parent{};

		GFX::SpritePacker sprPacker{};
		std::string spriteSheetPath{};
		SpriteSheet spriteSheet{};

		std::vector<Animation> animations;
		Animation* currentAnim{ nullptr };
		Layer* selectedLayer{ nullptr };

		f32 timelineFrame{};

		ivec2 canvasSize{ 1280, 720 };
		Color canvasColor = DefaultColors::White;
		i32 baseFPS{ 60 };
		vec2 viewPan{};
		f32 viewZoom{ 1.0f };

		bool playing = false;
		bool showMetricsWindow = false;
		bool showIDStackWindow = false;
		bool showKeyframeListWindow = false;

		Layer* layerToModify = nullptr;
		bool renameLayer = false;
		bool changeLayerSprite = false;

		std::vector<Layer>::iterator layerToDelete{};
		bool layerToDeleteIsSet = false;

		Impl(AnimEditor* parent) : parent{ parent }
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

			currentAnim = &animations.emplace_back();
			currentAnim->Name = "Animation 0";
			currentAnim->EndTime = 60;

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
			Gui::Text("Frame");
			Gui::SameLine();
			Gui::SetNextItemWidth(56.0f);
			Gui::DragFloat("##Timeline_FrameDrag", &timelineFrame, 1.0f, currentAnim->StartTime, currentAnim->EndTime, "%.0f", playing ? ImGuiSliderFlags_ReadOnly : 0);
			if (Gui::IsItemEdited() && !playing)
			{
				for (auto& layer : currentAnim->Layers)
					layer.CurrentEditTransform = GetTransformAtFrame(layer, timelineFrame);
			}

			Gui::SameLine();
			if (Gui::Button(playing ? "Stop" : "Play"))
			{
				playing = !playing;
				if (!playing)
				{
					for (auto& layer : currentAnim->Layers)
						layer.CurrentEditTransform = GetTransformAtFrame(layer, timelineFrame);
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
			InsertKeyframe(keyframes, static_cast<u32>(timelineFrame), value);
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
			auto layer = currentAnim->Layers.begin() + layerIndex;
			if (Gui::BeginPopupContextItem())
			{
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
					ImGuiSelectableFlags selectableFlags = (layerIndex == currentAnim->Layers.size() - 1) ? ImGuiSelectableFlags_Disabled : 0;
					if (Gui::Selectable("Move to the front", false, selectableFlags))
					{
						auto layer1 = currentAnim->Layers.begin() + layerIndex;
						auto layer2 = layer1 + 1;
						std::iter_swap(layer1, layer2);

						if (selectedLayer == &*layer1)
						{
							selectedLayer = &*layer2;
						}
					}

					selectableFlags = (layerIndex == 0) ? ImGuiSelectableFlags_Disabled : 0;
					if (Gui::Selectable("Move to the back", false, selectableFlags))
					{
						auto layer1 = currentAnim->Layers.begin() + layerIndex;
						auto layer2 = layer1 - 1;
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

		void LayerTreeNodeProperties(i32 layerIndex, std::vector<Layer>::reverse_iterator layer)
		{
			LayerContextMenu(layerIndex);

			const char* blendModeName_layer = BlendModeNames[static_cast<size_t>(layer->BlendMode)].data();

			Gui::SameLine();
			if (Gui::BeginCombo("##Layer_BlendMode", blendModeName_layer))
			{
				for (size_t i = 0; i < EnumCount<BlendMode>(); i++)
				{
					const char* blendModeName = BlendModeNames[i].data();
					if (Gui::Selectable(blendModeName))
						layer->BlendMode = static_cast<BlendMode>(i);
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
				ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX, ImGuiWindowFlags_MenuBar))
			{
				if (Gui::BeginMenuBar())
				{
					if (Gui::MenuItem("+L"))
					{
						size_t layerCount = currentAnim->Layers.size();
						char layerName[64]{};
						SDL_snprintf(layerName, sizeof(layerName) - 1, "Layer %llu", layerCount);

						const Sprite& defaultSprite = spriteSheet.GetSprite(0);
						Layer& newLayer = currentAnim->Layers.emplace_back();
						newLayer.Name = layerName;
						newLayer.BlendMode = BlendMode::Normal;
						newLayer.StartTime = timelineFrame;
						newLayer.EndTime = currentAnim->EndTime;
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

				i32 layerIndex = currentAnim->Layers.size() - 1;
				for (auto layer = currentAnim->Layers.rbegin(); layer != currentAnim->Layers.rend(); layer++)
				{
					Gui::PushID(layerIndex);
					ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

					if (selectedLayer == &*layer)
						nodeFlags |= ImGuiTreeNodeFlags_Selected;

					Gui::SetNextItemAllowOverlap();
					if (Gui::TreeNodeEx(layer->Name.c_str(), nodeFlags))
					{
						layer->Expanded = true;
						LayerTreeNodeProperties(layerIndex, layer);

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

					if (!layer->Expanded)
					{
						LayerTreeNodeProperties(layerIndex, layer);
					}

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
			if (currentAnim)
				SDL_snprintf(windowTitle, sizeof(windowTitle), "Timeline - \"%s\"###Timeline", currentAnim->Name.c_str());
			else
				SDL_snprintf(windowTitle, sizeof(windowTitle), "Timeline - No Animation###Timeline");

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
					auto& layers = currentAnim->Layers;
					for (auto layer = currentAnim->Layers.rbegin(); layer != currentAnim->Layers.rend(); layer++)
					{
						const vec2 rangePos = keyframePos;

						u32 layerStart = layer->StartTime - animStart;
						u32 layerEnd = layer->EndTime - animStart;
						TimelineRange(layer->Name.c_str(), layerStart, layerEnd, rangePos, rangeSize);
						layer->StartTime = layerStart + animStart;
						layer->EndTime = layerEnd + animStart;

						layer->StartTime = MathExtensions::Clamp<i32>(layer->StartTime, currentAnim->StartTime, currentAnim->EndTime - 1);
						layer->EndTime = MathExtensions::Clamp<i32>(layer->EndTime, layer->StartTime + 1, currentAnim->EndTime);

						if (layer->Expanded)
						{
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<vec2>(layer->Origin, keyframePos, 0, layerIndex);
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<vec2>(layer->Position, keyframePos, 1, layerIndex);
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<vec2>(layer->Size, keyframePos, 2, layerIndex);
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<f32>(layer->Rotation, keyframePos, 3, layerIndex);
							keyframePos.y += heightDelta;

							DrawLayerKeyframes<Color>(layer->Color, keyframePos, 4, layerIndex);
							keyframePos.y += heightDelta;

						}
						else
						{
							keyframePos.y += heightDelta;
						}

						layerIndex++;
						keyframePos.x = keyframesRegion.Min.x + horizontalPadding - timelineScroll;
					}

					drawList->PopClipRect();
					drawList->PopClipRect();
					ImGui::InvisibleButton("##Timeline_DopeSheet_KeyframeRegion", { frameLineDistance * animLength, 1.0f });

					// --- Current frame line
					const ImVec2 frameMarkerPos = ImVec2{ keyframesRegion.Min.x + timelineFrame * frameLineDistance + horizontalPadding - timelineScroll,
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

		char newAnimName[128]{};
		i32 newAnimTimings[2]{ 0, 60 };

		void NewAnimationModalWindow()
		{
			const ImVec2 center = Gui::GetMainViewport()->GetCenter();
			Gui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (Gui::BeginPopupModal("New Animation", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				Gui::Text("Name");
				Gui::SameLine();
				Gui::InputText("##NewAnimation_Name", newAnimName, sizeof(newAnimName) - 1);

				Gui::Text("Start and End Timings");
				Gui::SameLine();
				Gui::InputInt2("##NewAnimation_Timings", newAnimTimings);

				Gui::Separator();

				if (Gui::Button("Add", ImVec2(120.0f, 0.0f)))
				{
					Animation& newAnim = animations.emplace_back();

					newAnim.Name = newAnimName;
					newAnim.StartTime = newAnimTimings[0];
					newAnim.EndTime = newAnimTimings[1];

					currentAnim = &newAnim;
					Gui::CloseCurrentPopup();
				}

				Gui::SameLine();
					
				if (Gui::Button("Cancel", ImVec2(120.0f, 0.0f)))
					Gui::CloseCurrentPopup();

				Gui::EndPopup();
			}
		}

		void ResourcesWindow()
		{
			if (Gui::Begin("Resources"))
			{
				if (Gui::BeginTabBar("##Resources_TabBar"))
				{
					if (Gui::BeginTabItem("Animations"))
					{
						if (Gui::Button("+A"))
						{
							SDL_memset(newAnimName, 0, sizeof(newAnimName));
							newAnimTimings[0] = 0;
							newAnimTimings[1] = 60;
							Gui::OpenPopup("New Animation");
						}

						NewAnimationModalWindow();

						const ImVec2 contentRegion = Gui::GetContentRegionAvail();
						if (Gui::BeginListBox("##Resources_AnimationList", ImVec2(-FLT_MIN, contentRegion.y)))
						{
							for (auto& anim : animations)
							{
								const bool selected = &anim == currentAnim;

								if (Gui::Selectable(anim.Name.c_str(), &selected))
									currentAnim = &anim;
							}

							Gui::EndListBox();
						}
						Gui::EndTabItem();
					}
					if (Gui::BeginTabItem("Sprites"))
					{
						Gui::Text("fjiofjeoiwfjwoejfoewijoew sprites tab\njfiorjfew");
						Gui::EndTabItem();
					}
					Gui::EndTabBar();
				}
			}
			Gui::End();
		}

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
					currentAnim->Layers.erase(layerToDelete);
					layerToDeleteIsSet = false;
				};
				Gui::SameLine();
				if (Gui::Button("No")) { layerToDeleteIsSet = false; }
				Gui::End();
			}
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

			Layer* hoveredLayer = nullptr;
			auto& layers = currentAnim->Layers;
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

			if ((dragState.HeldMouseButtonsMask & (1 << ImGuiMouseButton_Left)) && selectedLayer != nullptr)
			{
				if (dragState.FavorOneAxis)
				{
					switch (dragState.AxisToFavor)
					{
					case DragAxis::Horizontal:
						selectedLayer->CurrentEditTransform.Position.x += dragState.DeltaMousePosition.x;
						break;
					case DragAxis::Vertical:
						selectedLayer->CurrentEditTransform.Position.y += dragState.DeltaMousePosition.y;
						break;
					}
				}
				else
				{
					selectedLayer->CurrentEditTransform.Position += dragState.DeltaMousePosition;
				}
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
				Xml::Element* frameElement = frameListElement->InsertNewChildElement(XmlElementNames::Keyframe);
				frameElement->SetAttribute(XmlElementNames::Keyframe_Frame, frame.Frame);
				Xml::SetAttribute(frameElement, XmlElementNames::Keyframe_Value, frame.Value);
			}
		}

		void WriteKeyframes_Xml(const std::vector<Keyframe<f32>>& keyframes, std::string_view elementName, Xml::Element* layerElement)
		{
			if (keyframes.empty())
				return;

			Xml::Element* frameListElement = layerElement->InsertNewChildElement(elementName.data());
			for (const auto& frame : keyframes)
			{
				Xml::Element* frameElement = frameListElement->InsertNewChildElement(XmlElementNames::Keyframe);
				frameElement->SetAttribute(XmlElementNames::Keyframe_Frame, frame.Frame);
				frameElement->SetAttribute(XmlElementNames::Keyframe_Value, frame.Value);
			}
		}

		template <typename T>
		void ReadKeyframes_Xml(std::vector<Keyframe<T>>& keyframes, std::string_view elementName, const Xml::Element* layerElement)
		{
			const Xml::Element* frameListElement = layerElement->FirstChildElement(elementName.data());
			if (frameListElement == nullptr)
				return;

			for (const Xml::Element* frameElement = frameListElement->FirstChildElement(XmlElementNames::Keyframe);
				frameElement;
				frameElement = frameElement->NextSiblingElement(XmlElementNames::Keyframe))
			{
				i32 frame = 0;
				if (frameElement->QueryIntAttribute(XmlElementNames::Keyframe_Frame, &frame) != 0)
					return;

				T value;
				const Xml::Attribute* valueAttrib = frameElement->FindAttribute(XmlElementNames::Keyframe_Value);
				Xml::TryGetValue(value, valueAttrib);

				keyframes.emplace_back(Keyframe<T>(frame, value));
			}
		}

		void ReadKeyframes_Xml(std::vector<Keyframe<f32>>& keyframes, std::string_view elementName, const Xml::Element* layerElement)
		{
			const Xml::Element* frameListElement = layerElement->FirstChildElement(elementName.data());
			if (frameListElement == nullptr)
				return;

			for (const Xml::Element* frameElement = frameListElement->FirstChildElement(XmlElementNames::Keyframe);
				frameElement;
				frameElement = frameElement->NextSiblingElement(XmlElementNames::Keyframe))
			{
				i32 frame = 0;
				f32 value = 0.0f;

				if (frameElement->QueryIntAttribute(XmlElementNames::Keyframe_Frame, &frame) != 0)
					return;

				if (frameElement->QueryFloatAttribute(XmlElementNames::Keyframe_Value, &value) != 0)
					return;

				keyframes.emplace_back(Keyframe<f32>(frame, value));
			}
		}

		void SaveFileDialog()
		{
			FileDialog saveFileDialog;
			saveFileDialog.Title = "Save as";

			if (!saveFileDialog.OpenSave())
				return;

			Xml::Document animSetDoc;
			Xml::Element* rootElement = animSetDoc.NewElement(XmlElementNames::AnimationSet);

			rootElement->SetAttribute(XmlElementNames::AnimationSet_Width, canvasSize.x);
			rootElement->SetAttribute(XmlElementNames::AnimationSet_Height, canvasSize.y);
			rootElement->SetAttribute(XmlElementNames::AnimationSet_FPS, baseFPS);
			Xml::SetAttribute(rootElement, XmlElementNames::AnimationSet_StageColor, canvasColor);
			rootElement->SetAttribute(XmlElementNames::AnimationSet_SpriteSheet, spriteSheetPath.c_str());

			for (const auto& anim : animations)
			{
				Xml::Element* animElement = rootElement->InsertNewChildElement(XmlElementNames::Animation);
				animElement->SetAttribute(XmlElementNames::Common_Name, anim.Name.c_str());
				animElement->SetAttribute(XmlElementNames::Common_Start, anim.StartTime);
				animElement->SetAttribute(XmlElementNames::Common_End, anim.EndTime);

				for (const auto& layer : anim.Layers)
				{
					Xml::Element* layerElement = animElement->InsertNewChildElement(XmlElementNames::AnimationLayer);
					layerElement->SetAttribute(XmlElementNames::Common_Name, layer.Name.c_str());
					layerElement->SetAttribute(XmlElementNames::AnimationLayer_Sprite, layer.Sprite->Name.c_str());

					size_t blendModeIndex = static_cast<size_t>(layer.BlendMode);
					layerElement->SetAttribute(XmlElementNames::AnimationLayer_BlendMode, BlendModeNames[blendModeIndex].data());

					layerElement->SetAttribute(XmlElementNames::Common_Start, static_cast<i32>(layer.StartTime));
					layerElement->SetAttribute(XmlElementNames::Common_End, static_cast<i32>(layer.EndTime));

					// Oh boy, I can't wait to start abusing the C++ type system!
					WriteKeyframes_Xml(layer.Origin, XmlElementNames::Keyframes_Origin, layerElement);
					WriteKeyframes_Xml(layer.Position, XmlElementNames::Keyframes_Position, layerElement);
					WriteKeyframes_Xml(layer.Size, XmlElementNames::Keyframes_Size, layerElement);
					WriteKeyframes_Xml(layer.Rotation, XmlElementNames::Keyframes_Rotation, layerElement);
					WriteKeyframes_Xml(layer.Color, XmlElementNames::Keyframes_Color, layerElement);
				}
			}

			animSetDoc.InsertFirstChild(rootElement);
			animSetDoc.SaveFile(saveFileDialog.OutputFilePath.c_str());
		}

		void OpenFileDialog()
		{
			FileDialog openFileDialog;
			openFileDialog.Title = "Open";

			if (!openFileDialog.OpenRead())
				return;

			currentAnim = nullptr;
			animations.clear();

			Xml::Document animSetDoc;
			if (!Xml::ParseFromFile(animSetDoc, openFileDialog.OutputFilePath));

			const Xml::Element* rootElement = animSetDoc.FirstChildElement(XmlElementNames::AnimationSet);
			rootElement->QueryIntAttribute(XmlElementNames::AnimationSet_Width, &canvasSize.x);
			rootElement->QueryIntAttribute(XmlElementNames::AnimationSet_Height, &canvasSize.y);
			rootElement->QueryIntAttribute(XmlElementNames::AnimationSet_FPS, &baseFPS);
			Xml::TryGetValue(canvasColor, rootElement->FindAttribute(XmlElementNames::AnimationSet_StageColor));

			const char* sprSheetPath{};
			if (rootElement->QueryAttribute(XmlElementNames::AnimationSet_SpriteSheet, &sprSheetPath))
				ImportSpritesFromFolder(sprSheetPath);

			for (const Xml::Element* animElement = rootElement->FirstChildElement(XmlElementNames::Animation);
				animElement;
				animElement = animElement->NextSiblingElement(XmlElementNames::Animation))
			{
				auto& anim = animations.emplace_back();
				if (currentAnim == nullptr)
					currentAnim = &anim;

				const char* animName{};
				if (animElement->QueryAttribute(XmlElementNames::Common_Name, &animName) == 0)
					anim.Name = animName;

				animElement->QueryUnsignedAttribute(XmlElementNames::Common_Start, &anim.StartTime);
				animElement->QueryUnsignedAttribute(XmlElementNames::Common_End, &anim.EndTime);

				for (const Xml::Element* layerElement = animElement->FirstChildElement(XmlElementNames::AnimationLayer);
					layerElement;
					layerElement = layerElement->NextSiblingElement(XmlElementNames::AnimationLayer))
				{
					Layer& layer = anim.Layers.emplace_back();

					const char* elementName = layerElement->Attribute(XmlElementNames::Common_Name);
					layer.Name = std::string(elementName);

					const char* refName{};
					if (layerElement->QueryAttribute(XmlElementNames::AnimationLayer_Sprite, &refName) == 0)
						layer.Sprite = &spriteSheet.GetSprite(refName);

					// TODO: Implement animation referencing

					const char* blendModeName{};
					if (layerElement->QueryAttribute(XmlElementNames::AnimationLayer_BlendMode, &blendModeName) == 0)
					{
						for (size_t i = 0; i < EnumCount<BlendMode>(); i++)
						{
							if (BlendModeNames[i] == blendModeName)
							{
								layer.BlendMode = static_cast<BlendMode>(i);
								break;
							}
						}
					}

					layerElement->QueryUnsignedAttribute(XmlElementNames::Common_Start, &layer.StartTime);
					layerElement->QueryUnsignedAttribute(XmlElementNames::Common_End, &layer.EndTime);

					ReadKeyframes_Xml(layer.Origin, XmlElementNames::Keyframes_Origin, layerElement);
					ReadKeyframes_Xml(layer.Position, XmlElementNames::Keyframes_Position, layerElement);
					ReadKeyframes_Xml(layer.Size, XmlElementNames::Keyframes_Size, layerElement);
					ReadKeyframes_Xml(layer.Rotation, XmlElementNames::Keyframes_Rotation, layerElement);
					ReadKeyframes_Xml(layer.Color, XmlElementNames::Keyframes_Color, layerElement);

					timelineFrame = 0.0f;
					layer.CurrentEditTransform = GetTransformAtFrame(layer, 0.0f);
				}
			}
		}

		void ImportSpritesFromFolder(std::string_view path)
		{
			if (!IO::Directory::Exists(path))
				return;

			spriteSheet.Destroy();

			sprPacker.AddFromDirectory(spriteSheetPath);
			sprPacker.Pack();

			spriteSheet.CreateFromSpritePacker(sprPacker);
			sprPacker.Clear();
		}

		void ImportSpritesFromFolderDialog()
		{
			FileDialog dialog;
			dialog.Title = "Import sprites from folder";

			if (!dialog.OpenDirectory())
				return;

			spriteSheetPath = IO::Path::GetNormalizedPath(dialog.OutputFilePath);
			ImportSpritesFromFolder(spriteSheetPath);
		}

		void MainMenu()
		{
			if (Gui::BeginMainMenuBar())
			{
				if (Gui::BeginMenu("File"))
				{
					if (Gui::MenuItem("New"))
					{
						currentAnim->Layers.clear();
						selectedLayer = nullptr;
						timelineFrame = 0.0f;
					}
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

					Gui::EndMenu();
				}

				if (Gui::BeginMenu("Debug"))
				{
					Gui::MenuItem("Metrics Window", nullptr, &showMetricsWindow);
					Gui::MenuItem("ID Stack Window", nullptr, &showIDStackWindow);
					Gui::MenuItem("Keyframe List Window", nullptr, &showKeyframeListWindow);
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
				
				Gui::DragInt2("##AnimSetProperties_Resolution", &canvasSize[0]);
				if (Gui::IsItemEdited())
				{
					canvasSize.x = MathExtensions::Clamp<i32>(canvasSize.x, ValueRanges::MinResolution.x, ValueRanges::MaxResolution.x);
					canvasSize.y = MathExtensions::Clamp<i32>(canvasSize.y, ValueRanges::MinResolution.y, ValueRanges::MaxResolution.y);
				}

				Gui::Text("FPS");
				Gui::SameLine();
				Gui::DragInt("##AnimSetProperties_FPS", &baseFPS, 1.0f, 30, 120);

				Gui::Text("Stage Color");
				Gui::SameLine();

				vec4 stageColor_vec4 = canvasColor.ToVector4();
				Gui::ColorEdit3("##AnimSetProperties_StageColor", &stageColor_vec4[0]);
				if (Gui::IsItemEdited())
					canvasColor = Color(stageColor_vec4);

				Gui::End();
			}
		}

		Layer* layerToDisplay = nullptr;
		void ShowKeyframeListWindow()
		{
			if (Gui::Begin("Keyframe List", &showKeyframeListWindow))
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
						if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Origin); }
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
						if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Position); }
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
						if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Size); }
						if (Gui::BeginTable("##FrameTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
						{
							Gui::TableSetupColumn("Frame");
							Gui::TableSetupColumn("Value");
							Gui::TableHeadersRow();
							if (layerToDisplay != nullptr)
							{
								for (const auto& keyframe : layerToDisplay->Size)
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
						if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Rotation); }
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
						if (Gui::Button("Sort")) { if (layerToDisplay != nullptr) SortKeyframes(layerToDisplay->Color); }
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

		void OnGUI()
		{
			DrawTimeline();
			//EasingPlotWindow();
			ResourcesWindow();

			AnimationSetPropertiesWindow();

			RenameLayerPopUp();
			ChangeLayerSpritePopUp();
			DeleteLayerPopUp();

			UpdateViewportInput();

			MainMenu();

			if (showMetricsWindow)
				Gui::ShowMetricsWindow(&showMetricsWindow);
			if (showIDStackWindow)
				Gui::ShowIDStackToolWindow(&showIDStackWindow);
			if (showKeyframeListWindow)
				ShowKeyframeListWindow();
		}

		ivec2 baseViewPan{};

		void DrawCanvas(SpriteRenderer* sprRenderer)
		{
			DragStateData& dragState = parent->DragState;

			if (dragState.HeldMouseButtonsMask & (1 << ImGuiMouseButton_Middle)) // Panning
			{
				viewPan += dragState.DeltaMousePosition;
			}

			const vec2 realCanvasSize = vec2(canvasSize) * viewZoom;

			sprRenderer->SetSpritePosition(viewPan);
			sprRenderer->SetSpriteSize(realCanvasSize);
			sprRenderer->SetSpriteColor(canvasColor);
			sprRenderer->PushSprite(nullptr);

			sprRenderer->PushOutlineRect(viewPan, realCanvasSize, {}, DefaultColors::Black);
		}

		void DrawAnimation(SpriteRenderer* sprRenderer)
		{
			BlendMode prevBlendMode{};
			for (const auto& layer : currentAnim->Layers)
			{
				if (timelineFrame < layer.StartTime || timelineFrame > layer.EndTime)
					continue;

				const Transform2D transform = GetTransformAtFrame(layer, timelineFrame);

				if (prevBlendMode != layer.BlendMode)
				{
					sprRenderer->RenderSprites(nullptr);
					sprRenderer->SetBlendMode(layer.BlendMode);
				}

				i32 texIndex{};

				sprRenderer->SpriteSheet().SetSpriteState(spriteSheet, *layer.Sprite, {}, &texIndex);
				sprRenderer->SetSpriteOrigin(transform.Origin);
				sprRenderer->SetSpritePosition(transform.Position + viewPan);
				sprRenderer->SetSpriteSize(transform.Size);
				sprRenderer->SetSpriteRotation(MathExtensions::ToRadians(transform.Rotation));
				sprRenderer->SetSpriteColor(transform.Color);
				sprRenderer->PushSprite(spriteSheet.GetTexture(texIndex));

				prevBlendMode = layer.BlendMode;
			}

			sprRenderer->RenderSprites(nullptr);
		}

		f32 easeTime{};
		void EasingTest(SpriteRenderer* sprRenderer)
		{
			easeTime += (1.0f / 60.0f);
			if (easeTime >= 1.0f)
				easeTime = 0.0f;

			f32 easedT = GetCubicBezierPoint(easingTime.x, easingTime.y, easeTime);
			f32 value = GetCubicBezierPoint(easingValue.x, easingValue.y, easedT);

			const vec2 sprPos(640.0f + value * 200.0f, 360.0f);

			sprRenderer->SpriteSheet().PushSprite(spriteSheet, 0, sprPos, vec2(1.0f), DefaultColors::White);
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
				timelineFrame = std::fmodf(timelineFrame, currentAnim->EndTime) + currentAnim->StartTime;
			}
			else
			{
				DragStateData& dragState = parent->DragState;

				BlendMode prevBlendMode{};
				for (const auto& layer : currentAnim->Layers)
				{
					if (timelineFrame < layer.StartTime || timelineFrame > layer.EndTime)
						continue;

					const Transform2D& currentTransform = layer.CurrentEditTransform;

					if (prevBlendMode != layer.BlendMode)
					{
						sprRenderer->RenderSprites(nullptr);
						sprRenderer->SetBlendMode(layer.BlendMode);
					}

					i32 texIndex{};
					sprRenderer->SpriteSheet().SetSpriteState(spriteSheet, *layer.Sprite, {}, &texIndex);
					sprRenderer->SetSpriteOrigin(currentTransform.Origin);
					sprRenderer->SetSpritePosition(currentTransform.Position + viewPan);
					sprRenderer->SetSpriteSize(currentTransform.Size);
					sprRenderer->SetSpriteRotation(MathExtensions::ToRadians(currentTransform.Rotation));
					sprRenderer->SetSpriteColor(currentTransform.Color);
					sprRenderer->PushSprite(spriteSheet.GetTexture(texIndex));

					prevBlendMode = layer.BlendMode;
				}
				
				sprRenderer->RenderSprites(nullptr);

				sprRenderer->SetBlendMode(BlendMode::Normal);
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

					if (dragState.HeldMouseButtonsMask != 0) // Dragging position text
					{
						char posText[64]{};
						const vec2 relMousePos = dragState.RelativeMousePosition;
						const vec2 absMousePos = dragState.AbsoluteMousePosition;

						SDL_snprintf(posText, sizeof(posText) - 1, "X: %+.1f\nY: %+.1f", relMousePos.x, relMousePos.y);
						sprRenderer->Font().PushString(font, posText, absMousePos, vec2(1.0f), DefaultColors::White);
					}

					sprRenderer->RenderSprites(nullptr);
				}
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

	i64 AnimEditor::GetStateID() const
	{
		return GameState_Main;
	}
}
