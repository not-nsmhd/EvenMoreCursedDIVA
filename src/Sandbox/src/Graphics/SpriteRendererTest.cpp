#include "SpriteRendererTest.h"
#include "Definitions.h"
#include "Rendering/Device.h"
#include "Rendering/Utilities.h"
#include "Rendering/Render2D/SpriteRenderer.h"
#include "GameContext.h"
#include "Animations/Animation.h"
#include "Animations/AnimationUtil.h"

using namespace Starshine;
using namespace Starshine::Rendering;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine::GFX;
using namespace Sandbox;

struct SpriteRendererTest::Impl
{
	std::unique_ptr<Texture> TestTexture{};
	SpriteRenderer* spriteRenderer{};

	Animations::AnimatedObject animObj;
	f32 animFrame{};

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

		spriteRenderer = GameContext::GetInstance()->SpriteRenderer.get();

		animObj.Position.X.KeyFrames.emplace_back(0, 0.0f, 0.5f, 1.0f);
		animObj.Position.Y.KeyFrames.emplace_back(0, 0.0f, 0.5f, 1.0f);

		animObj.Position.X.KeyFrames.emplace_back(30, 120.0f, 0.5f, 0.75f);
		animObj.Position.Y.KeyFrames.emplace_back(30, 120.0f, 0.5f, 0.75f);

		animObj.Position.X.KeyFrames.emplace_back(60, 240.0f);
		animObj.Position.Y.KeyFrames.emplace_back(60, 0.0f);

		animObj.Rotation.KeyFrames.emplace_back(0, 0.0f, 0.75f, 1.0f);
		animObj.Rotation.KeyFrames.emplace_back(30, 90.0f, 0.5f, 0.75f);
		animObj.Rotation.KeyFrames.emplace_back(60, 180.0f);
	}

	void Update(GameTime& gameTime)
	{
		animFrame += gameTime.ElapsedFrameTime.GetMilliseconds() / 16.6667f;
		if (animFrame >= 60.0f) { animFrame = 0.0f; }
	}

	void Draw(GameTime& gameTime)
	{
		Rendering::GetDevice()->Clear(ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);

		f32 xPos = Animations::GetValueAt(animObj.Position.X.KeyFrames, animFrame);
		f32 yPos = Animations::GetValueAt(animObj.Position.Y.KeyFrames, animFrame);

		f32 rotationDeg = Animations::GetValueAt(animObj.Rotation.KeyFrames, animFrame);

		spriteRenderer->SetSpritePosition(vec2{ xPos, yPos });
		spriteRenderer->SetSpriteRotation(glm::radians(rotationDeg));
		spriteRenderer->SetSpriteSize(vec2{ 128.0f, 128.0f });
		spriteRenderer->SetSpriteSource(RectangleF{ 0.0f, 0.0f, 1.0f, 1.0f });
		spriteRenderer->SetSpriteColor(DefaultColors::White);
		spriteRenderer->PushSprite(TestTexture.get());

		DrawDebugText();

		spriteRenderer->RenderSprites(nullptr);
	}

	void DrawDebugText()
	{
		std::array<char, 1024> debugText;
		::sprintf_s(debugText.data(), debugText.size() - 1, "Frame: %.3f", animFrame);

		const Font* font = GameContext::GetInstance()->DebugFont.get();
		spriteRenderer->Font().PushString(font, debugText.data(), {}, vec2(1.0f), DefaultColors::White);
	}
};

SpriteRendererTest::SpriteRendererTest() : impl(std::make_unique<Impl>())
{
}

SpriteRendererTest::~SpriteRendererTest()
{
}

bool SpriteRendererTest::Initialize()
{
	return true;
}

bool SpriteRendererTest::LoadContent()
{
	impl->LoadContent();
	return true;
}

void SpriteRendererTest::UnloadContent()
{
}

void SpriteRendererTest::Destroy()
{
}

void SpriteRendererTest::Update(GameTime& gameTime)
{
	impl->Update(gameTime);
}

void SpriteRendererTest::Draw(GameTime& gameTime)
{
	impl->Draw(gameTime);
}

i64 SpriteRendererTest::GetStateID() const
{
	return GameState_Editor;
}
