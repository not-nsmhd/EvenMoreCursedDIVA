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
	TestVertex { vec2{ 0.0f, 0.0f }, vec2{ 0.0f, 0.0f }, Color{ 255, 0, 0, 255 } },
	TestVertex { vec2{ 128.0f, 0.0f }, vec2{ 1.0f, 0.0f }, Color{ 0, 255, 0, 255 } },
	TestVertex { vec2{ 128.0f, 128.0f }, vec2{ 1.0f, 1.0f }, Color{ 0, 0, 255, 255 } },
	TestVertex { vec2{ 0.0f, 128.0f }, vec2{ 0.0f, 1.0f }, Color{ 255, 255, 255, 255 } },
};

static constexpr std::array<u16, 6> testIndexData
{
	0, 1, 2,
	2, 3, 0
};

static constexpr std::array<VertexAttrib, 3> testVertexDesc
{
	VertexAttrib { VertexAttribType::Position, 0, VertexAttribFormat::Float2, sizeof(TestVertex), offsetof(TestVertex, Position) },
	VertexAttrib { VertexAttribType::TexCoord, 0, VertexAttribFormat::Float2, sizeof(TestVertex), offsetof(TestVertex, TexCoord) },
	VertexAttrib { VertexAttribType::Color, 0, VertexAttribFormat::UnsignedByte4Norm, sizeof(TestVertex), offsetof(TestVertex, VtxColor) }
};

struct VertexShaderUniforms
{
	mat4 TransformMatrix{};
};

VertexShaderUniforms uniforms{};
static constexpr BlendStateDesc alphaBlendDesc 
{ 
	BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, 
	BlendFactor::Zero, BlendFactor::One, 
	BlendOperation::Add, 
	BlendOperation::Add
};

TestState::TestState()
{
}

TestState::~TestState()
{
}

bool TestState::Initialize()
{
	GFXDevice = Rendering::GetDevice();
	return true;
}

bool TestState::LoadContent()
{
	blendState = GFXDevice->CreateBlendState(alphaBlendDesc);

	testShader = Rendering::Utilities::LoadShader("diva/shaders/d3d11/VS_SpriteDefault.cso", "diva/shaders/d3d11/FS_SpriteDefault.cso");
	vertexDesc = GFXDevice->CreateVertexDesc(testVertexDesc.data(), testVertexDesc.size(), testShader.get());
	vertexBuffer = GFXDevice->CreateVertexBuffer(testVertexData.size() * sizeof(TestVertex), testVertexData.data(), false);
	indexBuffer = GFXDevice->CreateIndexBuffer(testIndexData.size() * sizeof(u16), IndexFormat::Index16bit, testIndexData.data(), false);
	uniformBuffer = GFXDevice->CreateUniformBuffer(sizeof(VertexShaderUniforms), nullptr, false);

	std::unique_ptr<u8[]> testTexData{};
	ivec2 texSize{};
	i32 texChannels{};

	Misc::ImageHelper::ReadImageFile("testfiles/test2.png", texSize, texChannels, testTexData);
	testTexture = GFXDevice->CreateTexture(texSize.x, texSize.y, TextureFormat::RGBA8, testTexData.get());
	testTexData = nullptr;

	return true;
}

void TestState::UnloadContent()
{
	blendState = nullptr;
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
	vertexDesc = nullptr;
	testShader = nullptr;
	testTexture = nullptr;
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
	RectangleF viewport = GFXDevice->GetViewportSize();
	uniforms.TransformMatrix = glm::transpose(glm::orthoRH_ZO(0.0f, viewport.Width, viewport.Height, 0.0f, 0.0f, 1.0f));
	uniformBuffer->SetData(&uniforms, 0, sizeof(VertexShaderUniforms));

	GFXDevice->Clear(ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);

	GFXDevice->SetBlendState(blendState.get());

	GFXDevice->SetShader(testShader.get());
	GFXDevice->SetVertexBuffer(vertexBuffer.get(), vertexDesc.get());
	GFXDevice->SetIndexBuffer(indexBuffer.get());

	GFXDevice->SetUniformBuffer(uniformBuffer.get(), ShaderStage::Vertex, 0);

	GFXDevice->SetTexture(testTexture.get(), 0);
	GFXDevice->DrawIndexed(PrimitiveType::Triangles, 0, 0, 6);

	GFXDevice->SwapBuffers();
}

std::string_view TestState::GetStateName() const
{
	return "Graphics Test State";
}
