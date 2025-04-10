#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../gfx/helpers/shader_helpers.h"
#include "../gfx/helpers/tex_helpers.h"
#include "gfx_backend_test.h"

namespace Testing
{
	bool GFXBackendTest::Initialize()
	{
		noBlend.srcColor = GFX::LowLevel::BlendFactor::BLEND_SRC_ALPHA;
		noBlend.srcAlpha = GFX::LowLevel::BlendFactor::BLEND_SRC_ALPHA;
		noBlend.dstColor = GFX::LowLevel::BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA;
		noBlend.dstAlpha = GFX::LowLevel::BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA;
		noBlend.colorOp = GFX::LowLevel::BlendOp::BLEND_OP_ADD;

		return true;
	}
	
	bool GFXBackendTest::LoadContent()
	{
		spriteRenderer.Initialize(graphicsBackend);
		return true;
	}
	
	void GFXBackendTest::UnloadContent()
	{
		spriteRenderer.Destroy();
	}
	
	void GFXBackendTest::Destroy()
	{
	}
	
	void GFXBackendTest::OnResize(u32 newWidth, u32 newHeight)
	{
	}
	
	void GFXBackendTest::Update()
	{
		
	}
	
	void GFXBackendTest::Draw()
	{
		graphicsBackend->Clear(GFX::LowLevel::ClearFlags::GFX_CLEAR_COLOR, Common::Color(0, 24, 24, 255), 1.0f, 0);
		graphicsBackend->SetBlendState(nullptr);

		float x = 0.0f;
		float y = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
		graphicsBackend->GetViewportSize(&x, &y, &width, &height);

		spriteRenderer.SetSpritePosition(vec2(0.0f, 0.0f));
		spriteRenderer.SetSpriteScale(vec2(16.0f, 16.0f));
		spriteRenderer.SetSpriteColor(Common::DefaultColors::White);
		spriteRenderer.PushSprite(nullptr);

		spriteRenderer.SetSpritePosition(vec2(width - 16.0f, 0.0f));
		spriteRenderer.SetSpriteScale(vec2(16.0f, 16.0f));
		spriteRenderer.SetSpriteColor(Common::DefaultColors::White);
		spriteRenderer.PushSprite(nullptr);

		spriteRenderer.SetSpritePosition(vec2(0.0f, height - 16.0f));
		spriteRenderer.SetSpriteScale(vec2(16.0f, 16.0f));
		spriteRenderer.SetSpriteColor(Common::DefaultColors::White);
		spriteRenderer.PushSprite(nullptr);

		spriteRenderer.SetSpritePosition(vec2(width - 16.0f, height - 16.0f));
		spriteRenderer.SetSpriteScale(vec2(16.0f, 16.0f));
		spriteRenderer.SetSpriteColor(Common::DefaultColors::White);
		spriteRenderer.PushSprite(nullptr);

		spriteRenderer.RenderSprites(nullptr);

		graphicsBackend->SwapBuffers();
	}
}
