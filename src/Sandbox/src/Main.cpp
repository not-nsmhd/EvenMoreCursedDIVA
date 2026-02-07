#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "Misc/ImageHelper.h"
#include "Rendering/Device.h"
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
	TestVertex { vec2{ 512.0f, 512.0f }, vec2{ 1.0f, 1.0f }, Color{ 0, 255, 0, 255 } },
	TestVertex { vec2{ 512.0f, 0.0f }, vec2{ 1.0f, 0.0f }, Color{ 0, 0, 255, 255 } },
	TestVertex { vec2{ 0.0f, 512.0f }, vec2{ 0.0f, 1.0f }, Color{ 255, 255, 255, 255 } },
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

class TestState : public GameState
{
public:
	TestState() {};

public:
	bool Initialize()
	{
		GFXDevice = Rendering::GetDevice();
		transformMatrix = glm::orthoRH_ZO(0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 1.0f);
		return true;
	}

	bool LoadContent()
	{
		vertexBuffer = GFXDevice->CreateVertexBuffer(testVertexData.size() * sizeof(TestVertex), testVertexData.data(), false);
		indexBuffer = GFXDevice->CreateIndexBuffer(testIndexData.size() * sizeof(u16), IndexFormat::Index16bit, testIndexData.data(), false);
		vertexDesc = GFXDevice->CreateVertexDesc(testVertexDesc.data(), testVertexDesc.size());
		testShader = Rendering::Utilities::LoadShader("diva/shaders/d3d9/VS_SpriteDefault.cso", "diva/shaders/d3d9/FS_SpriteDefault.cso");

		std::unique_ptr<u8[]> testTexData{};
		ivec2 texSize{};
		i32 texChannels{};
		Misc::ImageHelper::ReadImageFile("testfiles/test.png", texSize, texChannels, testTexData);

		testTexture = GFXDevice->CreateTexture(texSize.x, texSize.y, TextureFormat::RGBA8, false, false);
		testTexture->SetData(testTexData.get(), 0, 0, texSize.x, texSize.y);
		testTexData = nullptr;

		return true;
	}

	void UnloadContent()
	{
		vertexBuffer = nullptr;
		indexBuffer = nullptr;
		vertexDesc = nullptr;
		testShader = nullptr;
		testTexture = nullptr;
	}

	void Destroy()
	{
	}

	void Update(f64 deltaTime_milliseconds)
	{
		elapsedTime += deltaTime_milliseconds;
	}

	void Draw(f64 deltaTime_milliseconds)
	{
		GFXDevice->Clear(ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);

		GFXDevice->SetFaceCullingState(false, PolygonOrientation::Clockwise, Face::Back);
		GFXDevice->SetBlendState(true, BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One);
		GFXDevice->SetBlendOperation(BlendOperation::Add);

		GFXDevice->SetVertexBuffer(vertexBuffer.get(), vertexDesc.get());
		GFXDevice->SetIndexBuffer(indexBuffer.get());
		GFXDevice->SetShader(testShader.get());
		testShader->SetVertexShaderMatrix(0, transformMatrix);
		GFXDevice->SetTexture(testTexture.get(), 0);
		GFXDevice->DrawIndexed(PrimitiveType::Triangles, 0, 4, 6);

		GFXDevice->SwapBuffers();
	}

	std::string_view GetStateName() const { return "Test State"; }

private:
	f64 elapsedTime{};

	Rendering::Device* GFXDevice{};
	std::unique_ptr<Rendering::VertexBuffer> vertexBuffer{};
	std::unique_ptr<Rendering::IndexBuffer> indexBuffer{};
	std::unique_ptr<Rendering::VertexDesc> vertexDesc{};
	std::unique_ptr<Rendering::Shader> testShader{};
	std::unique_ptr<Rendering::Texture> testTexture{};

	mat4 transformMatrix{};
};

int SDL_main(int argc, char* argv[])
{
	GameInstance game;
	
	if (game.Initialize())
	{
		game.GetWindow()->SetTitle("Sandbox");
		game.SetState(std::make_unique<TestState>());
		game.EnterLoop();
		return 0;
	}

	return 1;
}
