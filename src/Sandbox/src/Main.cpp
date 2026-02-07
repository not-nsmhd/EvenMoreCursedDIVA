#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "GFX/SpritePacker.h"
#include "Rendering/Device.h"
#include "Rendering/Utilities.h"
#include "Rendering/Render2D/SpriteSheet.h"
#include "Rendering/Render2D/SpriteRenderer.h"
#include "Input/Keyboard.h"
#include "Input/Gamepad.h"
#include "Common/MathExt.h"

using namespace Starshine;
using namespace Starshine::GFX;
using namespace Starshine::Rendering;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine::Input;

struct TestVertex
{
	vec2 Position{};
	Color VtxColor{};
};

static constexpr std::array<TestVertex, 4> testVertexData
{
	TestVertex { vec2{ 0.0f, 0.0f }, Color{ 255, 0, 0, 255 } },
	TestVertex { vec2{ 1.0f, 1.0f }, Color{ 0, 255, 0, 255 } },
	TestVertex { vec2{ 1.0f, 0.0f }, Color{ 0, 0, 255, 255 } },
	TestVertex { vec2{ 0.0f, 1.0f }, Color{ 255, 255, 255, 255 } },
};

static constexpr std::array<u16, 6> testIndexData
{
	0, 3, 1,
	1, 2, 0
};

static constexpr std::array<VertexAttrib, 2> testVertexDesc
{
	VertexAttrib { VertexAttribType::Position, 0, VertexAttribFormat::Float2, sizeof(TestVertex), offsetof(TestVertex, Position) },
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
		return true;
	}

	bool LoadContent()
	{
		vertexBuffer = GFXDevice->CreateVertexBuffer(testVertexData.size() * sizeof(TestVertex), testVertexData.data(), false);
		indexBuffer = GFXDevice->CreateIndexBuffer(testIndexData.size() * sizeof(u16), IndexFormat::Index16bit, testIndexData.data(), false);
		vertexDesc = GFXDevice->CreateVertexDesc(testVertexDesc.data(), testVertexDesc.size());
		testShader = Rendering::Utilities::LoadShader("diva/shaders/d3d9/VS_Test.cso", "diva/shaders/d3d9/FS_Test.cso");
		return true;
	}

	void UnloadContent()
	{
		vertexBuffer = nullptr;
		indexBuffer = nullptr;
		vertexDesc = nullptr;
		testShader = nullptr;
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

		GFXDevice->SetVertexBuffer(vertexBuffer.get(), vertexDesc.get());
		GFXDevice->SetIndexBuffer(indexBuffer.get());
		GFXDevice->SetShader(testShader.get());
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
