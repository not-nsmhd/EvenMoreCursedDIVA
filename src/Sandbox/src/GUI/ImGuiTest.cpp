#include "ImGuiTest.h"
#include "Definitions.h"
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

	f32 timelineScroll = 0.0f;

	vec2 testPos{};
	
	vec2 viewportSize{ 1280.0f, 720.0f };
	vec2 viewPosOffset{ 0.0f, 0.0f };
	float viewScale{ 1.0f };

	ImVec2 viewportDragOffset{};

	f32 currentFrame{};

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

	void TestTimeline_Frames()
	{

	}

	void TestTimeline()
	{
		const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

		constexpr ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_NoCollapse;

		if (ImGui::Begin("Timeline", nullptr, windowFlags))
		{
			ImGui::SetNextItemWidth(32.0f);
			ImGui::DragFloat("Frame", &currentFrame, 1.0f, 0.0f, 100.0f, "%.0f");
			ImGui::SameLine();

			ImGui::Button("Play");
			
			ImGui::BeginGroup();
			{
				ImGui::BulletText("TestObject1");
				ImGui::BulletText("TestObject2");
				ImGui::BulletText("TestObject3");
			}
			ImGui::EndGroup();

			ImGui::End();
		}
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

		static constexpr float wheelScale = 1.0f / 33.3f;

		if (io.MouseWheel != 0.0f)
		{
			viewScale += io.MouseWheel * wheelScale;
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

	void MainMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("New", "Ctrl + N");

				ImGui::Separator();

				ImGui::MenuItem("Open", "Ctrl + O");
				ImGui::MenuItem("Save", "Ctrl + S");
				ImGui::MenuItem("Save as...", "Ctrl + Shift + S");

				ImGui::Separator();

				ImGui::MenuItem("Quit");
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void Update(f64 deltaTime_ms)
	{
	}

	void Draw(f64 deltaTime_ms)
	{
		Rendering::GetDevice()->Clear(ClearFlags_Color, DefaultColors::Gray, 1.0f, 0);

		TestViewport();
		TestTimeline();

		MainMenu();

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

i64 ImGuiTest::GetStateID() const
{
	return Sandbox::GameState_Editor;
}
