#include "RenderingTest.h"
#include "common/color.h"
#include "gfx/new/Renderer.h"
#include "io/File.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Starshine::Testing
{
	using namespace Starshine::GFX;
	using namespace Starshine::IO;
	using namespace Common;
	using std::string_view;

	struct TestVertex
	{
		vec2 Position;
		u8vec4 Color;
	};

	constexpr std::array<TestVertex, 4> TestVertexData =
	{
		TestVertex
		{ vec2(0.0f, 0.0f), u8vec4(255, 0, 0, 255) },
		{ vec2(128.0f, 128.0f), u8vec4(0, 255, 0, 255) },
		{ vec2(128.0f, 0.0f), u8vec4(0, 0, 255, 255) },
		{ vec2(0.0f, 128.0f), u8vec4(255, 255, 255, 255) }
	};

	constexpr std::array<u16, 6> TestIndexData =
	{
		0, 2, 1,
		1, 3, 0
	};

	constexpr std::array<VertexAttrib, 2> TestVertexDesc =
	{
		VertexAttrib
		{ VertexAttribType::Position, 0, VertexAttribFormat::Float, 2, false, sizeof(TestVertex), offsetof(TestVertex, Position) },
		{ VertexAttribType::Color, 0, VertexAttribFormat::UnsignedByte, 4, false, sizeof(TestVertex), offsetof(TestVertex, Color) }
	};

	struct RenderingTest::Impl
	{
		Renderer* renderer = nullptr;
		VertexBuffer* testVertexBuffer = nullptr;
		VertexDesc* testVertexDesc = nullptr;
		IndexBuffer* testIndexBuffer = nullptr;
		Shader* testShader = nullptr;

		mat4 TransformMatrix{};
		ShaderVariableIndex VS_TransformMatrix{};

		bool Initialize()
		{
			renderer = Renderer::GetInstance();
			TransformMatrix = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 1.0f);
			return true;
		}

		bool LoadContent()
		{
			testVertexBuffer = renderer->CreateVertexBuffer(TestVertexData.size() * sizeof(TestVertex), nullptr, true);
			testVertexBuffer->SetData((void*)TestVertexData.data(), 0, TestVertexData.size() * sizeof(TestVertex));

			testVertexDesc = renderer->CreateVertexDesc(TestVertexDesc.data(), TestVertexDesc.size());

			testIndexBuffer = renderer->CreateIndexBuffer(TestIndexData.size() * sizeof(u16), IndexFormat::Index16bit, (void*)TestIndexData.data(), false);

			testShader = renderer->LoadShaderFromXml("diva/shaders/Test_MatrixTransform1.xml");

			VS_TransformMatrix = testShader->GetVariableIndex("TransformMatrix");
			testShader->SetVariableValue(VS_TransformMatrix, &TransformMatrix);

			return true;
		}

		void Destroy()
		{
			renderer->DeleteResource(testVertexBuffer);
			renderer->DeleteResource(testVertexDesc);
			renderer->DeleteResource(testIndexBuffer);
			renderer->DeleteResource(testShader);

			testShader = nullptr;
			testVertexBuffer = nullptr;
			testVertexDesc = nullptr;
			testIndexBuffer = nullptr;
			renderer = nullptr;
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			renderer->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);

			renderer->SetVertexBuffer(testVertexBuffer);
			renderer->SetVertexDesc(testVertexDesc);
			renderer->SetIndexBuffer(testIndexBuffer);
			renderer->SetShader(testShader);

			renderer->DrawIndexed(PrimitiveType::Triangles, 0, 6);

			renderer->SwapBuffers();
		}
	};

	RenderingTest::RenderingTest() : impl(new RenderingTest::Impl())
	{
	}

	bool RenderingTest::Initialize()
	{
		return impl->Initialize();
	}

	bool RenderingTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void RenderingTest::UnloadContent()
	{
	}

	void RenderingTest::Destroy()
	{
		impl->Destroy();
		delete impl;
	}

	void RenderingTest::Update(f64 deltaTime_milliseconds)
	{
	}

	void RenderingTest::Draw(f64 deltaTime_milliseconds)
	{
		impl->Draw(deltaTime_milliseconds);
	}

	string_view RenderingTest::GetStateName() const
	{
		return "[Dev] Rendering Test";
	}
}
