#include "SpriteTest.h"
#include "common/color.h"
#include "common/math_ext.h"
#include "gfx/Renderer.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "io/File.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Starshine::Testing
{
	using namespace Starshine::GFX;
	using namespace Starshine::GFX::Render2D;
	using namespace Starshine::IO;
	using namespace Common;
	using std::string_view;

	struct SpriteTest::Impl
	{
		Renderer* BaseRenderer = nullptr;
		Texture* TestTexture = nullptr;

		SpriteRenderer* SpriteRenderer = nullptr;

		float Rotation = 0.0f;

		bool Initialize()
		{
			return true;
		}

		bool LoadContent()
		{
			BaseRenderer = Renderer::GetInstance();
			SpriteRenderer = new Render2D::SpriteRenderer();

			TestTexture = BaseRenderer->LoadTexture("diva/sprites/test2.png");

			return true;
		}

		void Destroy()
		{
			BaseRenderer->DeleteResource(TestTexture);
			SpriteRenderer->Destroy();
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			float diff = deltaTime_milliseconds / 16.6667f;
			Rotation += MathExtensions::ToRadians(diff);

			BaseRenderer->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);

			vec2 basePos = {};
			float radians = Rotation;

			for (size_t i = 0; i < 4; i++)
			{
				basePos.x = SDL_cosf(radians) * 132.0f;
				basePos.y = SDL_sinf(radians) * 132.0f;

				SpriteRenderer->SetSpritePosition(vec2(640.0f + basePos.x, 360.0f + basePos.y));
				SpriteRenderer->SetSpriteScale(vec2(128.0f, 128.0f));
				SpriteRenderer->SetSpriteOrigin(vec2(64.0f, 64.0f));
				SpriteRenderer->PushSprite(TestTexture);

				radians += MathExtensions::TwoPi / 4.0f;
			}

			SpriteRenderer->SetBlendMode(BlendMode::Add);
			SpriteRenderer->RenderSprites(nullptr);

			BaseRenderer->SwapBuffers();
		}
	};

	SpriteTest::SpriteTest() : impl(new SpriteTest::Impl())
	{
	}

	bool SpriteTest::Initialize()
	{
		return impl->Initialize();
	}

	bool SpriteTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void SpriteTest::UnloadContent()
	{
	}

	void SpriteTest::Destroy()
	{
		impl->Destroy();
		delete impl;
	}

	void SpriteTest::Update(f64 deltaTime_milliseconds)
	{
	}

	void SpriteTest::Draw(f64 deltaTime_milliseconds)
	{
		impl->Draw(deltaTime_milliseconds);
	}

	string_view SpriteTest::GetStateName() const
	{
		return "[Dev] Sprite Rendering Test";
	}
}
