#include <SDL2/SDL_main.h>
#include "GameInstance.h"
#include "Rendering/Device.h"
#include "IO/Path/File.h"
#include "Misc/ImageHelper.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace Starshine;
using namespace Starshine::Rendering;

struct TestVertex
{
	vec2 Position;
	vec2 TexCoord;
	Color Color;
};

static constexpr std::array<TestVertex, 4> testVertexData
{
	TestVertex { vec2 { 0.0f, 0.0f }, vec2{ 0.0f, 0.0f }, Color { 255, 0, 0 } },
	TestVertex { vec2 { 256.0f, 256.0f }, vec2{ 1.0f, 1.0f }, Color { 0, 255, 0 } },
	TestVertex { vec2 { 256.0f, 0.0f }, vec2{ 1.0f, 0.0f }, Color { 0, 0, 255 } },
	TestVertex { vec2 { 0.0f, 256.0f }, vec2{ 0.0f, 1.0f }, Color { 255, 255, 255 } }
};

static constexpr std::array<VertexAttrib, 3> testVertexAttribs
{
	VertexAttrib { VertexAttribType::Position, 0, VertexAttribFormat::Float, 2, false, sizeof(TestVertex), offsetof(TestVertex, Position) },
	VertexAttrib { VertexAttribType::TexCoord, 0, VertexAttribFormat::Float, 2, false, sizeof(TestVertex), offsetof(TestVertex, TexCoord) },
	VertexAttrib { VertexAttribType::Color, 0, VertexAttribFormat::UnsignedByte, 4, true, sizeof(TestVertex), offsetof(TestVertex, Color) }
};

static constexpr std::array<u16, 6> testIndexData
{
	0, 1, 2,
	0, 3, 1
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
		testVertexBuffer = GFXDevice->CreateVertexBuffer(testVertexData.size() * sizeof(TestVertex), testVertexData.data(), false);
		testIndexBuffer = GFXDevice->CreateIndexBuffer(testIndexData.size() * sizeof(u16), IndexFormat::Index16bit, testIndexData.data(), false);
		testVertexDesc = GFXDevice->CreateVertexDesc(testVertexAttribs.data(), testVertexAttribs.size());

		std::unique_ptr<u8[]> vpSource{};
		size_t vpSourceSize = IO::File::ReadAllBytes("diva/shaders/opengl/VS_SpriteDefault.vp", vpSource);

		std::unique_ptr<u8[]> fpSource{};
		size_t fpSourceSize = IO::File::ReadAllBytes("diva/shaders/opengl/FS_SpriteDefault.fp", fpSource);

		testShader = GFXDevice->LoadShader(vpSource.get(), vpSourceSize, fpSource.get(), fpSourceSize);

		vpSource = nullptr;
		fpSource = nullptr;

		std::unique_ptr<u8[]> testTexData{};
		ivec2 size{};
		i32 channels{};

		Misc::ImageHelper::ReadImageFile("testfiles/test.png", size, channels, testTexData);
		testTexture = GFXDevice->CreateTexture(size.x, size.y, GFX::TextureFormat::RGBA8, false, false);
		testTexture->SetData(testTexData.get(), 0, 0, size.x, size.y);

		testTexData = nullptr;

		GFXDevice->SetFaceCullingState(true, PolygonOrientation::CounterClockwise, Face::Back);
		GFXDevice->SetBlendState(true, BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One);

		return true;
	}

	void UnloadContent()
	{
		testTexture = nullptr;
		testIndexBuffer = nullptr;
		testVertexBuffer = nullptr;
		testVertexDesc = nullptr;
	}

	void Destroy()
	{
	}

	void Update(f64 deltaTime_milliseconds)
	{
	}

	void Draw(f64 deltaTime_milliseconds)
	{
		GFXDevice->Clear(ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);

		GFXDevice->SetVertexBuffer(testVertexBuffer.get());
		GFXDevice->SetIndexBuffer(testIndexBuffer.get());
		GFXDevice->SetVertexDesc(testVertexDesc.get());
		GFXDevice->SetShader(testShader.get());
		GFXDevice->SetTexture(testTexture.get(), 0);

		mat4 projMatrix = glm::orthoNO(0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 1.0f);
		testShader->SetVertexShaderMatrix(0, projMatrix);

		GFXDevice->DrawIndexed(PrimitiveType::Triangles, 0, 6);

		GFXDevice->SwapBuffers();
	}

	std::string_view GetStateName() const { return "Test State"; }

private:
	Rendering::Device* GFXDevice{};

	std::unique_ptr<VertexBuffer> testVertexBuffer{};
	std::unique_ptr<IndexBuffer> testIndexBuffer{};
	std::unique_ptr<VertexDesc> testVertexDesc{};
	std::unique_ptr<Shader> testShader{};
	std::unique_ptr<Texture> testTexture{};
};

int SDL_main(int argc, char* argv[])
{
	GameInstance game;
	
	if (game.Initialize())
	{
		game.SetState(std::make_unique<TestState>());
		game.EnterLoop();
		return 0;
	}

	return 1;
}
