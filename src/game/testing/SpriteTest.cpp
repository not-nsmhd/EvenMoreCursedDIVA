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

		Shader* CheckeboardShader = nullptr;
		ShaderVariableIndex Shader_CheckerboardSize = InvalidShaderVariable;

		SpriteRenderer* SpriteRenderer = nullptr;
		float DrawTime = 0.0f;

		bool Initialize()
		{
			return true;
		}

		bool LoadContent()
		{
			BaseRenderer = Renderer::GetInstance();
			SpriteRenderer = new Render2D::SpriteRenderer();

			TestTexture = BaseRenderer->LoadTexture("diva/sprites/test2.png", false, true);
			CheckeboardShader = BaseRenderer->LoadShaderFromXml("diva/shaders/SpriteCheckerboard.xml");
			Shader_CheckerboardSize = CheckeboardShader->GetVariableIndex("CheckerboardSize");

			return true;
		}

		void Destroy()
		{
			BaseRenderer->DeleteResource(TestTexture);
			BaseRenderer->DeleteResource(CheckeboardShader);
			SpriteRenderer->Destroy();
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			//BaseRenderer->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);
			DrawTime += deltaTime_milliseconds / 1000.0f;

			float checkerboardSize = SDL_sinf(DrawTime) / 2.0f + 0.5f;
			checkerboardSize *= 10.0f;
			checkerboardSize += 10.0f;
			CheckeboardShader->SetVariableValue(Shader_CheckerboardSize, &checkerboardSize);

			SpriteRenderer->SetSpritePosition(vec2(0.0f, 0.0f));
			SpriteRenderer->SetSpriteScale(vec2(1280.0f, 720.0f));
			SpriteRenderer->SetSpriteOrigin(vec2(0.0f, 0.0f));
			SpriteRenderer->SetSpriteColor(Color(64, 64, 64, 255));
			SpriteRenderer->PushSprite(nullptr);

			SpriteRenderer->RenderSprites(nullptr);

			SpriteRenderer->SetSpritePosition(vec2(0.0f, 0.0f));
			SpriteRenderer->SetSpriteScale(vec2(1280.0f, 720.0f));
			SpriteRenderer->SetSpriteOrigin(vec2(0.0f, 0.0f));
			SpriteRenderer->SetSpriteColor(Color(92, 92, 92, 255));
			SpriteRenderer->PushSprite(nullptr);

			SpriteRenderer->RenderSprites(CheckeboardShader);

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
