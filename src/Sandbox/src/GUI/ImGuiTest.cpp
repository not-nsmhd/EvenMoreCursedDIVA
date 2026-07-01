#include "ImGuiTest.h"
#include "Rendering/Device.h"
#include "Rendering/Utilities.h"
#include "GameContext.h"
#include "ImGui/Core/imgui.h"
#include "ImGui/Core/imgui_internal.h"
#include "ImGui/Extensions/StarshineExtensions.h"

#include "Rendering/D3D11/D3D11Texture.h"
#include "Rendering/D3D11/D3D11Framebuffer.h"
#undef LoadImage

using namespace Starshine;
using namespace Starshine::Rendering;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine::GFX;
using namespace Starshine::Rendering::D3D11;
using namespace Starshine::ImGuiExtensions;

struct ImGuiTest::Impl
{
	std::unique_ptr<Texture> TestTexture{};	

	vec2 testPos{};

	i32 currentTimelineFrame = 0;
	i32 timelineFrames = 100;
	static constexpr std::array<int, 4> keyframeLocations
	{
		4, 5, 6, 9
	};

	f32 timelineScroll = 0.0f;

	vec2 viewportSize{ 1280.0f, 720.0f };
	vec2 viewPosOffset{ 0.0f, 0.0f };
	float viewScale{ 1.0f };

	ImVec2 viewportDragOffset{};

	Impl()
	{
	}

	~Impl()
	{
	}

	void LoadContent()
	{
		Rendering::Utilities::LoadImage("testfiles/test.png", TestTexture);
		TestTexture->SetDebugName("TestTexture");
	}

	void TestPropsWindow()
	{
		if (ImGui::Begin("Testing Properties"))
		{
			ImGui::DragFloat2("Position", &testPos[0]);
		}
		ImGui::End();
	}

	void TestTimeline_Frames()
	{

	}

	void TestTimeline()
	{
		const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

		const ImVec2 windowSize = ImVec2(mainViewport->WorkSize.x, 256.0f);
		const ImVec2 windowPos = ImVec2(0.0f, mainViewport->WorkSize.y - windowSize.y);

		constexpr f32 headerHeight = 24.0f;
		constexpr f32 objectListWidth = 256.0f;

		constexpr f32 frameLineDistance = 16.0f;

		constexpr ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_NoCollapse;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});

		auto drawObjectInList = [](std::string_view name)
		{
			bool open = ImGui::TreeNodeEx(name.data());
			if (open)
			{
				ImGui::Text("Origin");
				ImGui::Text("Position");
				ImGui::Text("Scale");
				ImGui::Text("Rotation");
				ImGui::Text("Color");
				ImGui::Text("Sprite");
				ImGui::TreePop();
			}
		};

		if (ImGui::Begin(u8"Timeline", nullptr, windowFlags))
		{
			const ImVec2 contentRegion = ImGui::GetContentRegionAvail();
			const ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Frame");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(32.0f);
			ImGui::DragInt("##1", &currentTimelineFrame, 1.0f, 0, 100);

			ImGui::SameLine(objectListWidth);
			ImGui::BeginChild("##TestTimeline_DopeSheetHeader", {0.0f, 24.0f});
			{
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				const ImVec2 headerRegionStart = { ImGui::GetCursorScreenPos().x + itemSpacing.x, ImGui::GetCursorScreenPos().y + itemSpacing.y };
				const ImVec2 headerRegionEnd = { ImGui::GetContentRegionAvail().x - itemSpacing.x, ImGui::GetContentRegionAvail().y - itemSpacing.y };

				ImVec2 framePos = headerRegionStart;
				framePos.x += 4.0f - timelineScroll;

				ImVec2 currentFrameRectStart = { framePos.x - 4.0f, headerRegionStart.y };
				ImVec2 currentFrameRectEnd = { framePos.x + 5.0f, currentFrameRectStart.y + headerRegionEnd.y };

				std::array<char, 8> frameIndexText;

				for (int i = 0; i < 100; i++)
				{
					if (i % 5 == 0)
					{
						::sprintf_s(frameIndexText.data(), frameIndexText.size() - 1, "%d", i);
						ImVec2 textSize = ImGui::CalcTextSize(frameIndexText.data());
						drawList->AddText({ framePos.x - textSize.x / 2.0f, framePos.y }, GetU32StyleColor(ImGuiCol_PlotLines), frameIndexText.data());
					}
					else
					{
						drawList->AddLine(framePos, { framePos.x, framePos.y + headerRegionEnd.y }, GetU32StyleColor(ImGuiCol_PlotLines));
					}

					if (currentTimelineFrame == i)
					{
						drawList->AddRect(currentFrameRectStart, currentFrameRectEnd, GetU32StyleColor(ImGuiCol_PlotHistogramHovered));
					}

					framePos.x += frameLineDistance;
					currentFrameRectStart.x += frameLineDistance;
					currentFrameRectEnd.x += frameLineDistance;
				}
			}
			ImGui::EndChild();

			ImGui::BeginChild("##TestTimeline_ObjectList", { objectListWidth, 0.0f }, ImGuiChildFlags_FrameStyle);
			{
				drawObjectInList("HitEffect_Note");
				//drawObjectInList("HitEffect_Explosion");
				//drawObjectInList("HitEffect_Shockwave");
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("##TestTimeline_DopeSheet", {}, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_HorizontalScrollbar);
			{
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				const ImVec2 dopeSheetRegionStart = ImGui::GetCursorScreenPos();
				const ImVec2 dopeSheetRegionEnd = ImGui::GetContentRegionAvail();

				timelineScroll = ImGui::GetScrollX();

				ImVec2 frameLineStart = dopeSheetRegionStart;
				f32 frameLineVisibilityTestValue = frameLineStart.x - timelineScroll;
				for (int i = 0; i < 100; i++)
				{
					if ((frameLineVisibilityTestValue <= dopeSheetRegionStart.x + dopeSheetRegionEnd.x) && (frameLineVisibilityTestValue >= dopeSheetRegionStart.x))
					{
						u32 color = (i == currentTimelineFrame) ? GetU32StyleColor(ImGuiCol_PlotHistogramHovered) : GetU32StyleColor(ImGuiCol_Border);
						drawList->AddLine(frameLineStart, { frameLineStart.x, frameLineStart.y + dopeSheetRegionEnd.y }, color);
					}
					if (i < 99)
					{
						frameLineStart.x += frameLineDistance;
						frameLineVisibilityTestValue += frameLineDistance;
					}
				}

				for (auto it : keyframeLocations)
				{
					constexpr f32 radius = 6.0f;

					ImVec2 keyframeDisplayPos { it * frameLineDistance + dopeSheetRegionStart.x, 8.0f + dopeSheetRegionStart.y };
					f32 visibilityTestValue = keyframeDisplayPos.x + (radius * 2.0f) - timelineScroll;

					if ((visibilityTestValue <= dopeSheetRegionStart.x + dopeSheetRegionEnd.x) && (visibilityTestValue >= dopeSheetRegionStart.x))
					{
						drawList->AddCircleFilled(keyframeDisplayPos, radius, GetU32StyleColor(ImGuiCol_PlotLines), 4);
					}
				}

				ImGui::InvisibleButton("##TestTimeline_DopeSheetRegion", { frameLineStart.x - dopeSheetRegionStart.x, dopeSheetRegionEnd.y });
			}
			ImGui::EndChild();

#if 0
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 basePos = ImGui::GetCursorScreenPos();

			// Header
			ImGui::BeginChild("##TestTimeline_Header");
			{
				ImRect timelineHeaderRegion = ImRect(basePos.x, basePos.y, contentRegion.x, basePos.y + headerHeight);
				drawList->AddRectFilled(timelineHeaderRegion.GetTL(), timelineHeaderRegion.GetBR(), GetU32StyleColor(ImGuiCol_Tab));

				timelineHeaderRegion.Max.x = frameTypePanelWidth;
				drawList->AddRect(timelineHeaderRegion.GetTL(), timelineHeaderRegion.GetBR(), GetU32StyleColor(ImGuiCol_Border));

				ImVec2 frameLineStart = ImVec2(basePos.x + frameTypePanelWidth, basePos.y);
				ImVec2 frameLineEnd = ImVec2(frameLineStart.x, frameLineStart.y + headerHeight);

				std::array<char, 20> frameIndexText{};
				ImVec2 frameIndexTextPos = ImVec2(frameLineStart.x + 4.0f, frameLineStart.y + 2.0f);

				for (int i = 0; i < timelineFrames; i++)
				{
					drawList->AddLine(frameLineStart, frameLineEnd, GetU32StyleColor(ImGuiCol_PlotLines));

					frameLineStart.x += frameWidth;
					frameLineEnd.x += frameWidth;
					frameIndexTextPos.x += frameWidth;
				}

				basePos = timelineHeaderRegion.GetBL();
			}
			ImGui::EndChild();

			// Object List
			ImGui::BeginChild("##TestTimeline_ObjectList");
			{
				ImRect objectListRegion = ImRect(basePos.x, basePos.y, frameTypePanelWidth, basePos.y + contentRegion.y);
				drawList->AddRectFilled(objectListRegion.GetTL(), objectListRegion.GetBR(), GetU32StyleColor(ImGuiCol_WindowBg));
				drawList->AddRect(objectListRegion.GetTL(), objectListRegion.GetBR(), GetU32StyleColor(ImGuiCol_Border));

				ImGui::TreeNodeEx("Test");
			}
			ImGui::EndChild();
#endif
		}
		ImGui::End();
		ImGui::PopStyleVar(1);
	}

	void TestViewport()
	{
		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::IsKeyDown(ImGuiKey::ImGuiKey_MouseMiddle))
		{
			viewportDragOffset = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
		}
		if (ImGui::IsKeyReleased(ImGuiKey::ImGuiKey_MouseMiddle))
		{
			viewPosOffset.x += viewportDragOffset.x;
			viewPosOffset.y += viewportDragOffset.y;
			viewportDragOffset = {};
		}

		SpriteRenderer* sprRenderer = Sandbox::GameContext::GetInstance()->SpriteRenderer.get();
		vec2 mergedViewportPos = vec2{ viewPosOffset.x + viewportDragOffset.x, viewPosOffset.y + viewportDragOffset.y };

		// Viewport base
		sprRenderer->ResetSprite();
		sprRenderer->SetSpritePosition(mergedViewportPos);
		sprRenderer->SetSpriteSize(viewportSize * viewScale);
		sprRenderer->SetSpriteColor(DefaultColors::White);
		sprRenderer->PushSprite(nullptr);

		sprRenderer->PushOutlineRect(mergedViewportPos, viewportSize * viewScale, {}, DefaultColors::Black);
			
		// Objects
		sprRenderer->SetBlendMode(BlendMode::Normal);
		sprRenderer->SetSpritePosition(vec2{ testPos.x, testPos.y } + mergedViewportPos);
		sprRenderer->SetSpriteSize(vec2{ 128.0f, 128.0f } * viewScale);
		sprRenderer->SetSpriteSource(RectangleF{ 0.0f, 0.0f, 1.0f, 1.0f });
		sprRenderer->SetSpriteColor(DefaultColors::White);
		sprRenderer->PushSprite(TestTexture.get());

		sprRenderer->RenderSprites(nullptr);
	}

	void Update(f64 deltaTime_ms)
	{
	}

	void Draw(f64 deltaTime_ms)
	{
		Rendering::GetDevice()->Clear(ClearFlags_Color, DefaultColors::Gray, 1.0f, 0);

		TestViewport();
		//TestPropsWindow();
		TestTimeline();

		//ImGui::ShowMetricsWindow();
	}
};

ImGuiTest::ImGuiTest() : impl(std::make_unique<Impl>())
{
}

ImGuiTest::~ImGuiTest()
{
}

bool ImGuiTest::Initialize()
{
	return true;
}

bool ImGuiTest::LoadContent()
{
	impl->LoadContent();
	return true;
}

void ImGuiTest::UnloadContent()
{
}

void ImGuiTest::Destroy()
{
}

void ImGuiTest::Update(Starshine::GameTime& gameTime)
{
	impl->Update(gameTime.ElapsedFrameTime.GetMilliseconds());
}

void ImGuiTest::Draw(Starshine::GameTime& gameTime)
{
	impl->Draw(gameTime.ElapsedFrameTime.GetMilliseconds());
}

std::string_view ImGuiTest::GetStateName() const
{
	return "ImGui Test";
}
