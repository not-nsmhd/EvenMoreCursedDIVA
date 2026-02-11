#include "SpriteRendererTest.h"
#include "Rendering/Device.h"
#include "Rendering/Utilities.h"
#include "Rendering/Render2D/SpriteRenderer.h"

using namespace Starshine;
using namespace Starshine::Rendering;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine::GFX;

struct SpriteRendererTest::Impl
{
	std::unique_ptr<Texture> TestTexture{};
	std::unique_ptr<SpriteRenderer> spriteRenderer{};

	Impl()
	{
	}

	~Impl()
	{
	}

	void LoadContent()
	{
		TestTexture = Rendering::Utilities::LoadImage("testfiles/test.png");
		TestTexture->SetDebugName("TestTexture");
		spriteRenderer = std::make_unique<SpriteRenderer>();
	}

	void Update(f64 deltaTime_ms)
	{
	}

	void Draw(f64 deltaTime_ms)
	{
		Rendering::GetDevice()->Clear(ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);

		spriteRenderer->SetSpritePosition(vec2{ 64.0f, 64.0f });
		spriteRenderer->SetSpriteSize(vec2{ 128.0f, 128.0f });
		spriteRenderer->SetSpriteSource(RectangleF{ 0.0f, 0.0f, 1.0f, 1.0f });
		spriteRenderer->SetSpriteColor(DefaultColors::White);
		spriteRenderer->PushSprite(TestTexture.get());

		spriteRenderer->SetSpritePosition(vec2{ 96.0f, 96.0f });
		spriteRenderer->SetSpriteSize(vec2{ 128.0f, 128.0f });
		spriteRenderer->SetSpriteSource(RectangleF{ 0.0f, 0.0f, 1.0f, 1.0f });
		spriteRenderer->SetSpriteColor(DefaultColors::White);
		spriteRenderer->PushSprite(TestTexture.get());

		spriteRenderer->RenderSprites(nullptr);

		Rendering::GetDevice()->SwapBuffers();
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

void SpriteRendererTest::Update(f64 deltaTime_milliseconds)
{
	impl->Update(deltaTime_milliseconds);
}

void SpriteRendererTest::Draw(f64 deltaTime_milliseconds)
{
	impl->Draw(deltaTime_milliseconds);
}

std::string_view SpriteRendererTest::GetStateName() const
{
	return "Sprite Renderer Test";
}
