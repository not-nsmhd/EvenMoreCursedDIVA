#include "RNGTest.h"
#include "../GameContext.h"
#include <Random/RNG.h>
#include <Input/Keyboard.h>

using namespace Starshine;
using namespace Starshine::Graphics;
using Starshine::Rendering::Render2D::SpriteRenderer;
using namespace Starshine::Random;
using namespace Starshine::Input;

namespace DIVA::Sandbox
{
	struct squareDisp
	{
		f32 hue{};
		i32 dispChance{};
	};

	struct RNGTest::Impl
	{
		static constexpr size_t SquareAreaWidth = 17;
		static constexpr size_t SquareAreaHeight = 11;

		static constexpr f32 squareSize = 64.0f;
		static constexpr f32 squarePadding = 16.0f;

		RNG rng{};
		TimeSpan elapsedTime{};

		std::array<squareDisp, SquareAreaWidth * SquareAreaHeight> squareHues{};
		std::array<i32, SquareAreaWidth> consecutiveColumnCounts{};

		Impl()
		{
		}

		void regenerateSquareColors(size_t rangeStart, size_t rangeEnd)
		{
			i32 consecutiveCount{};

			i32 i = 0;
			for (auto it = squareHues.begin() + rangeStart; it != squareHues.begin() + rangeEnd; it++, i++)
			{
				if (i >= SquareAreaWidth)
				{
					i = 0;
					consecutiveCount = 0;
				}

				it->dispChance = rng.Next(144);
				if (it->dispChance <= 72)
				{
					if (consecutiveColumnCounts[i] >= 3)
					{
						consecutiveColumnCounts[i] = 0;
						it->dispChance = 144;
						continue;
					}

					if (consecutiveCount >= 3)
					{
						consecutiveCount = 0;
						it->dispChance = 144;
						continue;
					}

					it->hue = (rng.NextDouble(false) * 360.0f);

					consecutiveCount++;
					consecutiveColumnCounts[i]++;
				}
				else
				{
					consecutiveCount = 0;
					consecutiveColumnCounts[i] = 0;
				}
			}
		}

		void Init()
		{
			regenerateSquareColors(0, squareHues.size());
		}

		void Update(GameTime& gameTime)
		{
			elapsedTime += gameTime.ElapsedFrameTime;
			if (elapsedTime.GetSeconds() > 1.0f)
			{
				constexpr size_t lastLineIndex = SquareAreaHeight - 1;

				SDL_memcpy(&*squareHues.begin(), &*(squareHues.begin() + SquareAreaWidth),
					(SquareAreaWidth * lastLineIndex) * sizeof(squareDisp));

				regenerateSquareColors(SquareAreaWidth * lastLineIndex, SquareAreaWidth * lastLineIndex + SquareAreaWidth);
				elapsedTime = TimeSpan::Zero();
			}
		}

		void Draw()
		{
			using namespace Starshine::Rendering;

			auto gfxDevice = Rendering::GetDevice();
			auto sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();

			gfxDevice->Clear(ClearFlags_Color, DefaultColors::ClearColor_Menus, 1.0f, 0);

			f32 xOffset{};
			f32 yOffset{};
			i32 hCount{};
			for (const squareDisp& disp : squareHues)
			{
				if (hCount == SquareAreaWidth)
				{
					hCount = 0;
					xOffset = 0.0f;
					yOffset += squareSize + squarePadding;
				}

				f32 gYOffset = elapsedTime.GetSeconds() * (squareSize + squarePadding);
				if (disp.dispChance <= 72)
				{
					sprRenderer->SetSpritePosition(vec2(squarePadding + xOffset, squarePadding + yOffset - gYOffset));
					sprRenderer->SetSpriteSize(vec2(squareSize));
					sprRenderer->SetSpriteColor(Color::FromHSV(SDL_fmodf(disp.hue, 360.0f), 1.0f, 1.0f));
					sprRenderer->PushSprite(nullptr);
				}

				xOffset += squareSize + squarePadding;
				hCount++;
			}

			sprRenderer->Present();
		}
	};
	
	RNGTest::RNGTest()
	{
	}

	RNGTest::~RNGTest()
	{
	}

	bool RNGTest::Initialize()
	{
		GameInstance->GetWindow()->SetTitle("Testing");
		impl = std::make_unique<Impl>();
		impl->Init();
		return true;
	}

	bool RNGTest::LoadContent()
	{
		return true;
	}

	void RNGTest::UnloadContent()
	{
	}

	void RNGTest::Destroy()
	{
		GameInstance->GetWindow()->SetTitle("Even More Cursed DIVA");
		impl = nullptr;
	}

	void RNGTest::Update(Starshine::GameTime& gameTime)
	{
		impl->Update(gameTime);

		if (Keyboard::IsKeyTapped(SDLK_ESCAPE))
			GameInstance->Quit();
	}

	void RNGTest::Draw(Starshine::GameTime& gameTime)
	{
		impl->Draw();
	}
}
