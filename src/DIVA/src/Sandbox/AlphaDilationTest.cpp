#include "AlphaDilationTest.h"
#include "../GameContext.h"
#include <Graphics/Texture.h>
#include <Rendering/Utilities.h>
#include <Misc/ImageHelper.h>
#include <Input/Keyboard.h>

using namespace Starshine;
using namespace Starshine::Graphics;
using namespace Starshine::Misc;
using namespace Starshine::Input;

namespace DIVA::Sandbox
{
	struct AlphaDilationTest::Impl
	{
		std::unique_ptr<Texture> testTexture{};

		Impl()
		{
		}

		void DilateImageData(Color* pixels, ivec2 size)
		{
			const i32 centerX = size.x / 2;

			i32 yPassUp = 0;
			i32 yPassDown = size.y;

			Color lastVisibleColor = pixels[0];
			auto setOrAssignPixelValue = [&](i32 x, i32 y)
			{
				Color* curPixel = &pixels[y * size.x + x];
				if (curPixel->A > 0)
				{
					lastVisibleColor = *curPixel;
					if (yPassUp == 0)
						yPassUp = y;
					else
						yPassDown = y;
				}
				else
				{
					curPixel->R = lastVisibleColor.R;
					curPixel->G = lastVisibleColor.G;
					curPixel->B = lastVisibleColor.B;
				}
			};

			// Horizontal pass
			for (i32 y = 0; y < size.y; y++)
			{
				// Sub-pass 1
				for (i32 x = centerX; x >= 0; x--)
					setOrAssignPixelValue(x, y);

				// Sub-pass 2
				for (i32 x = centerX; x < size.x; x++)
					setOrAssignPixelValue(x, y);
			}

			// Vertical pass
			for (i32 x = 0; x < size.x; x++)
			{
				for (i32 y = yPassUp; y >= 0; y--)
					setOrAssignPixelValue(x, y);

				for (i32 y = yPassDown; y < size.y; y++)
					setOrAssignPixelValue(x, y);
			}
		}

		bool LoadDilated()
		{
			ivec2 size{};
			i32 channels{};
			std::unique_ptr<u8[]> data{};
			if (!ImageHelper::ReadImageFile("sprites/devtest_dilation1.png", size, channels, data))
				return false;

			DilateImageData(reinterpret_cast<Color*>(data.get()), size);

			testTexture = std::make_unique<Texture>(size, TextureFormat::RGBA8, TextureFlags{}, data.get());

			return true;
		}

		bool LoadContent()
		{
			if (!LoadDilated())
				return false;

			Rendering::Utilities::UploadTextureToGPU(testTexture.get());

			return true;
		}

		void Draw()
		{
			using namespace Starshine::Rendering;

			auto gfxDevice = Rendering::GetDevice();
			auto sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();

			gfxDevice->Clear(ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);
			sprRenderer->SetBlendMode(BlendMode::Normal);

			sprRenderer->SetSpritePosition(vec2(320.0f, 16.0f));
			sprRenderer->SetSpriteSize(vec2(testTexture->GetSize()) * 2.0f);
			sprRenderer->SetSpriteColor(DefaultColors::White);
			sprRenderer->SetSpriteRotation(0.6f);
			sprRenderer->PushSprite(testTexture.get());

			//sprRenderer->RenderSprites(nullptr);

			// ------------------

			sprRenderer->SetBlendMode(BlendMode::Disabled);

			sprRenderer->SetSpritePosition(vec2(480.0f, 16.0f));
			sprRenderer->SetSpriteSize(vec2(testTexture->GetSize()) * 2.0f);
			sprRenderer->SetSpriteColor(DefaultColors::White);
			sprRenderer->PushSprite(testTexture.get());

			//sprRenderer->RenderSprites(nullptr);
		}
	};
	
	AlphaDilationTest::AlphaDilationTest()
	{
	}

	AlphaDilationTest::~AlphaDilationTest()
	{
	}

	bool AlphaDilationTest::Initialize()
	{
		impl = std::make_unique<Impl>();
		return true;
	}

	bool AlphaDilationTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void AlphaDilationTest::UnloadContent()
	{
	}

	void AlphaDilationTest::Destroy()
	{
		impl = nullptr;
	}

	void AlphaDilationTest::Update(Starshine::GameTime& gameTime)
	{
		if (Keyboard::IsKeyTapped(SDLK_ESCAPE))
			GameInstance->Quit();
	}

	void AlphaDilationTest::Draw(Starshine::GameTime& gameTime)
	{
		impl->Draw();
	}
}
