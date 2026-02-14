#include "ImGuiTest.h"
#include "Rendering/Device.h"
#include "Rendering/Utilities.h"
#include "Rendering/Render2D/SpriteRenderer.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

using namespace Starshine;
using namespace Starshine::Rendering;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine::GFX;

struct ImGuiTest::Impl
{
	std::unique_ptr<Texture> TestTexture{};
	std::unique_ptr<SpriteRenderer> spriteRenderer{};

	vec2 testPos{};

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
		spriteRenderer = std::make_unique<SpriteRenderer>();
	}

	void TestPropsWindow()
	{
		if (ImGui::Begin("Testing Properties"))
		{
			ImGui::DragFloat2("Position", &testPos[0]);
			ImGui::End();
		}
	}

	void TestHitbox()
	{
		constexpr ImGuiWindowFlags overlayWindowFlags = 
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoSavedSettings;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		if (ImGui::Begin("Testing Overlay", nullptr, overlayWindowFlags))
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			ImVec2 boxStart = ImVec2(testPos.x - 4.0f, testPos.y - 4.0f);
			ImVec2 boxEnd = ImVec2(testPos.x + 68.0f, testPos.y + 68.0f);

			ImVec2 mousePos = ImGui::GetMousePos();
			bool mouseInsideHitbox = ImRect(boxStart, boxEnd).Contains(mousePos);

			ImU32 boxColor = IM_COL32(255, 255, 255, 64);
			if (mouseInsideHitbox)
			{
				boxColor = IM_COL32(255, 0, 0, 64);
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
					testPos.x = dragDelta.x;
					testPos.y = dragDelta.y;
				}
			}

			boxStart = ImVec2(testPos.x - 4.0f, testPos.y - 4.0f);
			boxEnd = ImVec2(testPos.x + 68.0f, testPos.y + 68.0f);

			drawList->AddRectFilled(boxStart, boxEnd, boxColor);
			drawList->AddRect(boxStart, boxEnd, IM_COL32(255, 255, 255, 255));
			ImGui::End();
		}
	}

	void Update(f64 deltaTime_ms)
	{
		TestHitbox();
		TestPropsWindow();
	}

	void Draw(f64 deltaTime_ms)
	{
		Rendering::GetDevice()->Clear(ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);

		spriteRenderer->SetSpritePosition(testPos);
		spriteRenderer->SetSpriteSize(vec2{ 64.0f, 64.0f });
		spriteRenderer->SetSpriteSource(RectangleF{ 0.0f, 0.0f, 1.0f, 1.0f });
		spriteRenderer->SetSpriteColor(DefaultColors::White);
		spriteRenderer->PushSprite(TestTexture.get());

		spriteRenderer->RenderSprites(nullptr);
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

void ImGuiTest::Update(f64 deltaTime_milliseconds)
{
	impl->Update(deltaTime_milliseconds);
}

void ImGuiTest::Draw(f64 deltaTime_milliseconds)
{
	impl->Draw(deltaTime_milliseconds);
}

std::string_view ImGuiTest::GetStateName() const
{
	return "ImGui Test";
}
