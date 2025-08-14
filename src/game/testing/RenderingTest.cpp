#include "RenderingTest.h"
#include "common/color.h"
#include "gfx/new/Renderer.h"

namespace Starshine::Testing
{
	using namespace Starshine::GFX;
	using namespace Common;
	using std::string_view;

	struct RenderingTest::Impl
	{
		Renderer* renderer = nullptr;

		bool Initialize()
		{
			renderer = Renderer::GetInstance();
			return true;
		}

		void Destroy()
		{
			renderer = nullptr;
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			renderer->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);
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
		return true;
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
