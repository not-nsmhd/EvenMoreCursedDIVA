#include "SpriteCommandTest.h"
#include "../GameContext.h"
#include <Graphics/Texture.h>
#include <Rendering/Utilities.h>
#include <Misc/ImageHelper.h>
#include <Input/Keyboard.h>

using namespace Starshine;
using namespace Starshine::Graphics;
using Starshine::Rendering::Render2D::SpriteRenderer;
using namespace Starshine::Misc;
using namespace Starshine::Input;

namespace DIVA::Sandbox
{
	struct SpriteCommandTest::Impl
	{
		std::unique_ptr<Texture> testTexture{};
		f32 sineInput{};
		bool displayReport{};

		Impl()
		{
		}

		bool LoadContent()
		{
			if (!ImageHelper::ReadImageFile("sprites/devtest.png", testTexture))
				return false;

			Rendering::Utilities::UploadTextureToGPU(testTexture.get());
			return true;
		}

		void Update(GameTime& gameTime)
		{
			sineInput += gameTime.ElapsedFrameTime.GetSeconds() * 5.0f;
			sineInput = SDL_fmod(sineInput, MathExtensions::TwoPi);

			if (Keyboard::IsKeyTapped(SDLK_F10))
				displayReport = !displayReport;
		}

		inline f32 sin01(f32 f)
		{
			return SDL_sinf(f) / 2.0f + 0.5f;
		}

		void TestBasicSprites(SpriteRenderer* sprRenderer)
		{
			for (size_t i = 0; i < 6; i++)
			{
				const f32 sineInputOffset = i * MathExtensions::Pi;

				const f32 x = i * 128.0f + 16.0f;
				const f32 y = sin01(sineInput + sineInputOffset) * 64.0f + 16.0f;

				sprRenderer->SetSpritePosition(vec2(x, y));
				sprRenderer->SetSpriteSize(vec2(testTexture->GetSize()));
				sprRenderer->SetSpriteColor(DefaultColors::White);
				sprRenderer->PushSprite(testTexture.get());
			}
		}

		void TestText(SpriteRenderer* sprRenderer)
		{
			auto& fontRenderer = sprRenderer->Font();
			auto font = GameContext::GetInstance()->DefaultFont.get();

			fontRenderer.DrawString(font, "0123456789 ABCDEF abcdef", vec2(256.0f), vec2(1.5f), DefaultColors::White);
			fontRenderer.DrawString(font, "Line 1 (after text)", vec2(256.0f, 300.0f), vec2(1.0f), DefaultColors::White);

			sprRenderer->SetSpritePosition(vec2(640.0f, 360.0f));
			sprRenderer->SetSpriteSize(vec2(testTexture->GetSize()));
			sprRenderer->SetSpriteColor(DefaultColors::White);
			sprRenderer->PushSprite(testTexture.get());

			fontRenderer.DrawString(font, "Line 2 (after sprite)", vec2(256.0f, 350.0f), vec2(1.0f), DefaultColors::White);
			fontRenderer.DrawString(font, u8"\n初音ミク (after text)", vec2(256.0f, 350.0f), vec2(1.0f), DefaultColors::White);
		}

		void TestBlendModes(SpriteRenderer* sprRenderer)
		{
			sprRenderer->SetBlendMode(BlendMode::Add);
			sprRenderer->SetSpritePosition(vec2(640.0f, 480.0f));
			sprRenderer->SetSpriteSize(vec2(testTexture->GetSize()));
			sprRenderer->SetSpriteColor(DefaultColors::White);
			sprRenderer->PushSprite(testTexture.get());

			sprRenderer->SetBlendMode(BlendMode::Mulitply);
			sprRenderer->SetSpritePosition(vec2(780.0f, 480.0f));
			sprRenderer->SetSpriteSize(vec2(testTexture->GetSize()));
			sprRenderer->SetSpriteColor(DefaultColors::White);
			sprRenderer->PushSprite(testTexture.get());

			sprRenderer->SetBlendMode(BlendMode::Normal);
		}

		void TestTriangleStrip(SpriteRenderer* sprRenderer)
		{
			using Rendering::Render2D::SpriteVertex;
			using Rendering::PrimitiveType;

			const Color vertexColors[4]
			{
				DefaultColors::Red,
				DefaultColors::Green,
				DefaultColors::Blue,
				DefaultColors::Yellow
			};

			SpriteVertex stripVertices[6]{};
			const auto generateVertices = [&](vec2 basePos, vec2 triangleSize)
			{
				for (i32 i = 0; i < 6; i++)
				{
					const bool isEven = i % 2 == 0;

					SpriteVertex& vertex = stripVertices[i];
					vertex.Position = vec2(basePos.x + (i * triangleSize.x), basePos.y + (isEven ? 0.0f : triangleSize.y));
					vertex.Color = vertexColors[i % 4];
				}
			};

			generateVertices(vec2(860.0f, 70.0f), vec2(50.0f, 80.0f));
			sprRenderer->DrawShape(stripVertices, 6, PrimitiveType::TriangleStrip, nullptr);

			generateVertices(vec2(860.0f, 200.0f), vec2(50.0f, 80.0f));
			sprRenderer->DrawShape(stripVertices, 6, PrimitiveType::TriangleStrip, nullptr);
		}

		void DisplaySpriteRendererReport(SpriteRenderer* sprRenderer)
		{
			auto& fontRenderer = sprRenderer->Font();
			auto font = GameContext::GetInstance()->DebugFont.get();

			if (!displayReport)
			{
				fontRenderer.DrawString(font, "Press F10 to display command list", vec2(0.0f), vec2(1.0f),
					DefaultColors::White, DefaultColors::Black);
				return;
			}

			using Rendering::Render2D::CommandType;

			Rendering::Render2D::DrawCommand* commands = nullptr;
			size_t commandCount = sprRenderer->GetDrawCommands(&commands);

			char text[1024]{};
			i32 offset = 0;

			offset += SDL_snprintf(text + offset, sizeof(text) - offset, "Commands: %llu/4096", commandCount);
			for (size_t i = 0; i < commandCount; i++)
			{
				const auto* command = &commands[i];
				const char* commandName = Rendering::Render2D::CommandTypeNames[static_cast<size_t>(command->Type)];
				
				if (command->Type == CommandType::DrawSprites)
				{
					offset += SDL_snprintf(text + offset, sizeof(text) - offset, "\nCommand %llu: DrawSprites(%u, %u)",
						i, command->DrawSprites.FirstSpriteIndex, command->DrawSprites.SpriteCount);
				}
				else
					offset += SDL_snprintf(text + offset, sizeof(text) - offset, "\nCommand %llu: %s", i, commandName);
			}

			fontRenderer.DrawString(font, text, vec2(0.0f), vec2(1.0f), DefaultColors::White, DefaultColors::Black);
		}

		void Draw()
		{
			using namespace Starshine::Rendering;

			auto gfxDevice = Rendering::GetDevice();
			auto sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();

			gfxDevice->Clear(ClearFlags_Color, Color(0x64, 0x95, 0xED, 0xFF), 1.0f, 0);

			TestBasicSprites(sprRenderer);
			TestText(sprRenderer);
			TestBlendModes(sprRenderer);
			TestTriangleStrip(sprRenderer);

			DisplaySpriteRendererReport(sprRenderer);

			sprRenderer->Present();
		}
	};
	
	SpriteCommandTest::SpriteCommandTest()
	{
	}

	SpriteCommandTest::~SpriteCommandTest()
	{
	}

	bool SpriteCommandTest::Initialize()
	{
		GameInstance->GetWindow()->SetTitle("Testing");
		impl = std::make_unique<Impl>();
		return true;
	}

	bool SpriteCommandTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void SpriteCommandTest::UnloadContent()
	{
	}

	void SpriteCommandTest::Destroy()
	{
		GameInstance->GetWindow()->SetTitle("Even More Cursed DIVA");
		impl = nullptr;
	}

	void SpriteCommandTest::Update(Starshine::GameTime& gameTime)
	{
		impl->Update(gameTime);

		if (Keyboard::IsKeyTapped(SDLK_ESCAPE))
			GameInstance->Quit();
	}

	void SpriteCommandTest::Draw(Starshine::GameTime& gameTime)
	{
		impl->Draw();
	}
}
