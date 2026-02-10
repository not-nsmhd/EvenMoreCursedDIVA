#include "GraphicsTest.h"
#include "Misc/ImageHelper.h"
#include "Rendering/Utilities.h"
#include "Common/MathExt.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace Starshine;
using namespace Starshine::GFX;
using namespace Starshine::Rendering;

struct TestVertex
{
	vec2 Position{};
	vec2 TexCoord{};
	Color VtxColor{};
};

static constexpr std::array<TestVertex, 4> testVertexData
{
	TestVertex { vec2{ 0.0f, 0.0f }, vec2{ 0.0f, 0.0f }, Color{ 255, 255, 255, 255 } },
	TestVertex { vec2{ 256.0f, 256.0f }, vec2{ 1.0f, 1.0f }, Color{ 255, 255, 255, 255 } },
	TestVertex { vec2{ 256.0f, 0.0f }, vec2{ 1.0f, 0.0f }, Color{ 255, 255, 255, 255 } },
	TestVertex { vec2{ 0.0f, 256.0f }, vec2{ 0.0f, 1.0f }, Color{ 255, 255, 255, 255 } },
};

static constexpr std::array<u16, 6> testIndexData
{
	0, 3, 1,
	1, 2, 0
};

static constexpr std::array<VertexAttrib, 3> testVertexDesc
{
	VertexAttrib { VertexAttribType::Position, 0, VertexAttribFormat::Float2, sizeof(TestVertex), offsetof(TestVertex, Position) },
	VertexAttrib { VertexAttribType::TexCoord, 0, VertexAttribFormat::Float2, sizeof(TestVertex), offsetof(TestVertex, TexCoord) },
	VertexAttrib { VertexAttribType::Color, 0, VertexAttribFormat::UnsignedByte4Norm, sizeof(TestVertex), offsetof(TestVertex, VtxColor) }
};

bool TestState::Initialize()
{
	GFXDevice = Rendering::GetDevice();
	return true;
}

bool TestState::LoadContent()
{
	//vertexBuffer = GFXDevice->CreateVertexBuffer(testVertexData.size() * sizeof(TestVertex), testVertexData.data(), false);
	//indexBuffer = GFXDevice->CreateIndexBuffer(testIndexData.size() * sizeof(u16), IndexFormat::Index16bit, testIndexData.data(), false);
	//vertexDesc = GFXDevice->CreateVertexDesc(testVertexDesc.data(), testVertexDesc.size());
	//testShader = Rendering::Utilities::LoadShader("diva/shaders/opengl/VS_SpriteDefault.glsl", "diva/shaders/opengl/FS_SpriteDefault.glsl");

	//std::unique_ptr<u8[]> testTexData{};
	ivec2 texSize{};
	i32 texChannels{};
	//Misc::ImageHelper::ReadImageFile("testfiles/test2.png", texSize, texChannels, testTexData);

	//testTexture = GFXDevice->CreateTexture(texSize.x, texSize.y, TextureFormat::RGBA8, false, false);
	//testTexture->SetData(testTexData.get(), 0, 0, texSize.x, texSize.y);
	//testTexData = nullptr;

	return true;
}

void TestState::UnloadContent()
{
}

void TestState::Destroy()
{
}

void TestState::Update(f64 deltaTime_milliseconds)
{
	elapsedTime += deltaTime_milliseconds;
}

void TestState::Draw(f64 deltaTime_milliseconds)
{
	//RectangleF viewport = GFXDevice->GetViewportSize();
	//transformMatrix = glm::orthoRH_NO(0.0f, viewport.Width, viewport.Height, 0.0f, 0.0f, 1.0f);

	GFXDevice->Clear(ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);

	//GFXDevice->SetFaceCullingState(false, PolygonOrientation::CounterClockwise);
	//GFXDevice->SetBlendState(true, BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One);
	//GFXDevice->SetBlendOperation(BlendOperation::Add);

	//GFXDevice->SetVertexBuffer(vertexBuffer.get(), vertexDesc.get());
	//GFXDevice->SetIndexBuffer(indexBuffer.get());
	//GFXDevice->SetShader(testShader.get());
	//testShader->SetVertexShaderMatrix(0, transformMatrix);
	//GFXDevice->SetTexture(testTexture.get(), 0);
	//GFXDevice->DrawIndexed(PrimitiveType::Triangles, 0, 4, 6);

	GFXDevice->SwapBuffers();
}

std::string_view TestState::GetStateName() const
{
	return "Graphics Test State";
}
