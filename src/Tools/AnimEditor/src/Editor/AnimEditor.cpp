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
	struct Keyframe
	{
		f32 Frame{};
		f32 Value{};
	};

	struct KeyframeColor
	{
		f32 Frame{};
		Color Value{};
	};

	struct KeyframeVec2
	{
		f32 Frame{};
		vec2 Value{};
	};

	struct Layer
	{
		std::string Name;

		f32 StartTime{};
		f32 Duration{};

		std::vector<KeyframeVec2> Origin;
		std::vector<KeyframeVec2> Position;
		std::vector<KeyframeVec2> Scale;
		std::vector<Keyframe> Rotation;
		std::vector<KeyframeColor> Color;
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

		f32 timelineFrame{};
		f32 animLength{ 60.0f };

		ivec2 canvasSize{ 1280, 720 };
		vec2 viewPan{};
		f32 viewZoom{ 1.0f };

		vec2 currentPos{ 640.0f, 360.0f };
		vec2 currentSize{ 128.0f, 128.0f };
		vec2 currentOrigin{ 64.0f, 64.0f };
		f32 currentRotation{ 0.0f };
		Color currentColor{ DefaultColors::White };

		vec2 drawPos{};

		bool playing = false;
		bool showMetricsWindow = false;

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

		void InsertKeyframe(std::vector<Keyframe>& keyframes, f32 frame, f32 value)
		{
			if (keyframes.size() == 0)
			{
				Keyframe& keyframe = keyframes.emplace_back();

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

		void InsertKeyframe(std::vector<KeyframeVec2>& keyframes, f32 frame, const vec2& value)
		{
			if (keyframes.size() == 0)
			{
				KeyframeVec2& keyframe = keyframes.emplace_back();

				keyframe.Frame = frame;
				keyframe.Value = value;
				return;
			}

			size_t kfCount = keyframes.size();
			auto placement = keyframes.begin();

			for (size_t i = 0; i < kfCount; i++, placement++)
			{
				if (i == kfCount) { break; }

				KeyframeVec2& kf = keyframes[i];
				if ((i32)kf.Frame == (i32)frame)
				{
					kf.Value = value;
					return;
				}
				if (frame > kf.Frame)
				{
					continue;
				}

				keyframes.emplace(placement, KeyframeVec2{ frame, value });
				return;
			}

			keyframes.push_back({ frame, value });
		}

		void InsertKeyframe(std::vector<KeyframeColor>& keyframes, f32 frame, const Color& value)
		{
			if (keyframes.size() == 0)
			{
				KeyframeColor& keyframe = keyframes.emplace_back();

				keyframe.Frame = frame;
				keyframe.Value = value;
				return;
			}

			size_t kfCount = keyframes.size();
			auto placement = keyframes.begin();

			for (size_t i = 0; i < kfCount; i++, placement++)
			{
				if (i == kfCount) { break; }

				KeyframeColor& kf = keyframes[i];
				if ((i32)kf.Frame == (i32)frame)
				{
					kf.Value = value;
					return;
				}
				if (frame > kf.Frame)
				{
					continue;
				}

				keyframes.emplace(placement, KeyframeColor{ frame, value });
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

						Layer& newLayer = layers.emplace_back();
						newLayer.Name = layerName;
					}
					if (Gui::BeginItemTooltip())
					{
						Gui::TextUnformatted("Add a new layer");
						Gui::EndTooltip();
					}
				}
				Gui::EndMenuBar();

				auto addPropFields_vec2 = [&](const std::string_view name, const std::string_view baseID, vec2& value)
				{
					Gui::Text(name.data());
					char idBuffer[64]{};

					SDL_snprintf(idBuffer, sizeof(idBuffer) - 1, "%s_Drag", baseID.data());

					Gui::SameLine();
					Gui::SetNextItemWidth(56.0f * 2.0f);
					Gui::DragFloat2(idBuffer, &value.x, 1.0f, 0.0f, 0.0f, "%.1f");

					SDL_snprintf(idBuffer, sizeof(idBuffer) - 1, "%s_AddFrameButton", baseID.data());

					Gui::SameLine();
					Gui::PushID(idBuffer);
					if (Gui::Button("+")) {/* InsertKeyframe(timelineFrame, currentValue); */ }
					Gui::PopID();
				};

				auto addPropFields_f32 = [&](const std::string_view name, const std::string_view baseID, f32& value)
				{
					Gui::Text(name.data());
					char idBuffer[64]{};

					SDL_snprintf(idBuffer, sizeof(idBuffer) - 1, "%s_Drag", baseID.data());

					Gui::SameLine();
					Gui::SetNextItemWidth(56.0f * 2.0f);
					Gui::DragFloat(idBuffer, &value, 1.0f, 0.0f, 0.0f, "%.1f");

					SDL_snprintf(idBuffer, sizeof(idBuffer) - 1, "%s_AddFrameButton", baseID.data());

					Gui::SameLine();
					Gui::PushID(idBuffer);
					if (Gui::Button("+")) {/* InsertKeyframe(timelineFrame, currentValue); */ }
					Gui::PopID();
				};

				auto addPropFields_color = [&](const std::string_view name, const std::string_view baseID, Color& value)
				{
					Gui::Text(name.data());
					char idBuffer[64]{};

					SDL_snprintf(idBuffer, sizeof(idBuffer) - 1, "%s_Edit", baseID.data());

					vec4 colorVec4 = value.ToVector4();
					Gui::SameLine();
					Gui::SetNextItemWidth(56.0f * 2.0f);
					Gui::ColorEdit4(idBuffer, &colorVec4.r);
					value = Color(colorVec4);

					SDL_snprintf(idBuffer, sizeof(idBuffer) - 1, "%s_AddFrameButton", baseID.data());

					Gui::SameLine();
					Gui::PushID(idBuffer);
					if (Gui::Button("+")) {/* InsertKeyframe(timelineFrame, currentValue); */ }
					Gui::PopID();
				};

				i32 layerIndex = 0;
				for (auto& layer : layers)
				{
					Gui::PushID(layerIndex++);
					if (Gui::TreeNode("", layer.Name.c_str()))
					{
						//layerExpanded = true;
						addPropFields_vec2("Origin", "##Layer0_Origin", currentOrigin);
						addPropFields_vec2("Position", "##Layer0_Position", currentPos);
						addPropFields_vec2("Size", "##Layer0_Size", currentSize);
						addPropFields_f32("Rotation", "##Layer0_Rotation", currentRotation);
						addPropFields_color("Color", "##Layer0_Color", currentColor);

						Gui::TreePop();
					}
					Gui::PopID();
				}
			}
			Gui::EndChild();
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
					static constexpr f32 frameLineDistance = 15.0f;

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

					/*for (auto& keyframe : keyframes)
					{
						const ImVec2 framePos = { keyframesRegion.Min.x + keyframe.Frame * frameLineDistance + padding.x - scroll,
							keyframesRegion.Min.y + layerListStartHeight };
						drawList->AddCircleFilled(framePos, 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLines]), 4);
					}*/

					auto drawKeyframePoints_vec2 = [&](const std::vector<KeyframeVec2> frames, f32 yPos)
					{
						for (auto& keyframe : frames)
						{
							const ImVec2 framePos = { keyframesRegion.Min.x + keyframe.Frame * frameLineDistance + padding.x - scroll,
								keyframesRegion.Min.y + yPos };
							drawList->AddCircleFilled(framePos, 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLines]), 4);
						}
					};

					auto drawKeyframePoints_f32 = [&](const std::vector<Keyframe> frames, f32 yPos)
					{
						for (auto& keyframe : frames)
						{
							const ImVec2 framePos = { keyframesRegion.Min.x + keyframe.Frame * frameLineDistance + padding.x - scroll,
								keyframesRegion.Min.y + yPos };
							drawList->AddCircleFilled(framePos, 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLines]), 4);
						}
					};

					auto drawKeyframePoints_color = [&](const std::vector<KeyframeColor> frames, f32 yPos)
					{
						for (auto& keyframe : frames)
						{
							const ImVec2 framePos = { keyframesRegion.Min.x + keyframe.Frame * frameLineDistance + padding.x - scroll,
								keyframesRegion.Min.y + yPos };
							drawList->AddCircleFilled(framePos, 6.0f, Gui::GetColorU32(style.Colors[ImGuiCol_PlotLines]), 4);
						}
					};

					/*if (layerExpanded)
					{
						f32 propStartHeight = layerListStartHeight + style.FontSizeBase;

						/*drawKeyframePoints_vec2(layers[0].Origin, propStartHeight);
						propStartHeight += style.FontSizeBase;

						drawKeyframePoints_vec2(layers[0].Position, propStartHeight);
						propStartHeight += style.FontSizeBase;

						drawKeyframePoints_vec2(layers[0].Scale, propStartHeight);
						propStartHeight += style.FontSizeBase;

						drawKeyframePoints_f32(layers[0].Rotation, propStartHeight);
						propStartHeight += style.FontSizeBase;

						drawKeyframePoints_color(layers[0].Color, propStartHeight);
					}*/

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

		void DrawObjectGizmos(SpriteRenderer* sprRenderer)
		{
			if (playing || Gui::IsAnyItemFocused()) { return; }

			auto& io = Gui::GetIO();
			auto debugFont = GameContext::GetInstance()->DebugFont.get();

			const vec2 pannedPos = currentPos + viewPan;

			const RectangleF sprRect(pannedPos.x - currentOrigin.x, pannedPos.y - currentOrigin.y, currentSize.x, currentSize.y);
			bool hovered = sprRect.Contains(io.MousePos.x, io.MousePos.y);

			if (io.MouseDown[0])
			{
				if (baseMousePosNotSet)
				{
					baseMousePosNotSet = false;
					baseMousePos = io.MousePos;

					if (hovered)
					{
						basePoint = currentPos;
					}
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

				if (hovered)
				{
					if (axisToFavor == 0)
					{
						currentPos.x = basePoint.x + mouseDrag.x;
						currentPos.y = basePoint.y;
					}
					else if (axisToFavor == 1)
					{
						currentPos.x = basePoint.x;
						currentPos.y = basePoint.y + mouseDrag.y;
					}
					else
					{
						currentPos.x = basePoint.x + mouseDrag.x;
						currentPos.y = basePoint.y + mouseDrag.y;
					}
				}
			}
			if (io.MouseReleased[0])
			{
				mouseDrag = {};
				baseMousePos = {};

				if (hovered)
				{
					basePoint = {};
					axisToFavor = -1;
				}

				baseMousePosNotSet = true;
			}

			u8 alpha = hovered ? 128 : 0;

			const vec2 realPos = sprRect.Position();

			// Base rect
			sprRenderer->SetSpritePosition(realPos);
			sprRenderer->SetSpriteSize(sprRect.Size());
			sprRenderer->SetSpriteColor({ 255, 0, 0, alpha });
			sprRenderer->PushSprite(nullptr);

			sprRenderer->PushOutlineRect(realPos, sprRect.Size(), {}, DefaultColors::Red);

			// Origin axes
			sprRenderer->SetSpritePosition(vec2{ realPos.x, realPos.y + currentOrigin.y }); // X axis
			sprRenderer->SetSpriteSize(vec2{ currentSize.x, 1.0f });
			sprRenderer->SetSpriteColor(DefaultColors::Red);
			sprRenderer->PushSprite(nullptr);

			sprRenderer->SetSpritePosition(vec2{ realPos.x + currentOrigin.x, realPos.y }); // Y axis
			sprRenderer->SetSpriteSize(vec2{ 1.0f, currentSize.y });
			sprRenderer->SetSpriteColor(DefaultColors::Red);
			sprRenderer->PushSprite(nullptr);

			// Offset info text
			if (io.MouseDown[0] && hovered)
			{
				char textBuf[128];
				SDL_snprintf(textBuf, sizeof(textBuf) - 1, "X: %.1f\nY: %.1f", mouseDrag.x, mouseDrag.y);
				sprRenderer->Font().PushString(debugFont, textBuf, { io.MousePos.x, io.MousePos.y }, vec2(1.0f), DefaultColors::White);
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
				timelineFrame = std::fmodf(timelineFrame, 100.0f);
				
				drawPos = GetKeyframeValue(timelineFrame);
			}
			else
			{
				drawPos = currentPos;
			}

			auto sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();

			DrawCanvas(sprRenderer);

			i32 texIndex{};
			sprRenderer->SpriteSheet().SetSpriteState(spriteSheet, 0, {}, &texIndex);
			sprRenderer->SetSpritePosition(currentPos + viewPan);
			sprRenderer->SetSpriteSize(currentSize);
			sprRenderer->SetSpriteOrigin(currentOrigin);
			sprRenderer->SetSpriteRotation(currentRotation);
			sprRenderer->SetSpriteColor(currentColor);
			sprRenderer->PushSprite(spriteSheet.GetTexture(texIndex));

			DrawObjectGizmos(sprRenderer);

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
