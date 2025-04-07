#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gfx/helpers/shader_helpers.h"
#include "gfx/helpers/tex_helpers.h"
#include "gfx_backend_test.h"

namespace Testing
{
	struct TestVertex
	{
		glm::vec2 position;
		glm::u8vec4 color;
		glm::vec2 texCoord;
	};

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
		TestVertex vertexData[] = 
		{
			{ glm::vec2(0.0f, 0.0f), glm::u8vec4(255, 255, 255, 255), glm::vec2(0.0f, 0.0f) },
			{ glm::vec2(128.0f, 128.0f), glm::u8vec4(255, 255, 255, 64), glm::vec2(1.0f, 1.0f) },
			{ glm::vec2(128.0f, 0.0f), glm::u8vec4(255, 255, 255, 255), glm::vec2(1.0f, 0.0f) },
			{ glm::vec2(0.0f, 128.0f), glm::u8vec4(255, 255, 255, 64), glm::vec2(0.0f, 1.0f) }
		};

		u16 indexData[] = 
		{
			0, 3, 1,
			1, 2, 0
		};

		shader = GFX::Helpers::LoadShaderFromDescriptor(graphicsBackend, "shaders/SpriteDefault.xml");
		vertexDesc = graphicsBackend->CreateVertexDescription(TestVertexAttribs, 3, sizeof(TestVertex), shader);

		vertexBuffer = graphicsBackend->CreateVertexBuffer(GFX::LowLevel::BufferUsage::BUFFER_USAGE_STATIC, vertexData, sizeof(vertexData));
		indexBuffer = graphicsBackend->CreateIndexBuffer(GFX::LowLevel::BufferUsage::BUFFER_USAGE_STATIC, GFX::LowLevel::IndexFormat::INDEX_16BIT, 
			indexData, sizeof(indexData));

		texture = GFX::Helpers::LoadTexture(graphicsBackend, "sprites/test.png");

		projMatrix = glm::orthoLH_ZO(0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 1.0f);

		return true;
	}
	
	void GFXBackendTest::UnloadContent()
	{
		graphicsBackend->DestroyBuffer(vertexBuffer);
		graphicsBackend->DestroyBuffer(indexBuffer);
		graphicsBackend->DestroyVertexDescription(vertexDesc);
		graphicsBackend->DestroyShader(shader);
		graphicsBackend->DestroyTexture(texture);
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

		graphicsBackend->SetVertexDescription(vertexDesc);
		graphicsBackend->BindVertexBuffer(vertexBuffer);
		graphicsBackend->BindIndexBuffer(indexBuffer);
		graphicsBackend->BindShader(shader);
		graphicsBackend->SetShaderMatrix(0, &projMatrix);
		graphicsBackend->BindTexture(texture, 0);
		graphicsBackend->DrawIndexed(GFX::LowLevel::PrimitiveType::PRIMITIVE_TRIANGLES, 4, 0, 6);

		graphicsBackend->SwapBuffers();
	}
}
