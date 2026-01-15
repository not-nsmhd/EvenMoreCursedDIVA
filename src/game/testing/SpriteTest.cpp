#include "SpriteTest.h"
#include <Common/Types.h>
#include <Common/MathExt.h>
#include "gfx/Renderer.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "gfx/Render2D/SpriteSheet.h"
#include "input/Keyboard.h"

#include <GFX/SpritePacker.h>
#include <IO/Path/Directory.h>
#include <IO/Path/File.h>
#include <IO/Xml.h>
#include <vector>

#include "util/logging.h"

namespace Starshine::Testing
{
	using namespace Starshine::GFX;
	using namespace Starshine::GFX::Render2D;
	using namespace Starshine::IO;
	using namespace Starshine::Input;

	struct SpriteTest::Impl
	{
		SpriteRenderer* SpriteRenderer = nullptr;
		Font TestFont;

		SpritePacker sprPacker;
		std::vector<Texture*> sprTextures;

		size_t textureCount = 1;
		i32 displayedTextureIndex = 0;
		bool drawSpriteBorders = false;

		bool Initialize()
		{
			sprPacker.Initialize();
			return true;
		}

		bool LoadContent()
		{
			SpriteRenderer = new Render2D::SpriteRenderer();
			TestFont.ReadBMFont("diva/fonts/debug.fnt");

			Directory::IterateFiles("diva/sprites/devtest2", 
				[&](std::string_view filePath)
				{
					sprPacker.AddImage(filePath);
				});

			sprPacker.Pack();
			textureCount = sprPacker.GetTextureCount();

			sprTextures.reserve(textureCount);
			for (size_t i = 0; i < textureCount; i++)
			{
				const SheetTextureInfo* texInfo = sprPacker.GetTextureInfo(i);
				if (texInfo != nullptr)
				{
					Texture* gpuTex = Renderer::GetInstance()->CreateTexture(texInfo->Size.x, texInfo->Size.y, TextureFormat::RGBA8, false, true);
					gpuTex->SetData(0, 0, texInfo->Size.x, texInfo->Size.y, texInfo->Data.get());
					sprTextures.push_back(gpuTex);
				}
			}

#if 0
			Xml::Printer xmlPrinter = Xml::Printer();
			xmlPrinter.OpenElement("SpriteSheet");
			{
				for (size_t i = 0; i < sprPacker.GetSpriteCount(); i++)
				{
					const SpriteInfo* sprite = sprPacker.GetSpriteInfo(i);
					if (!sprite->WasPacked) { continue; }

					xmlPrinter.OpenElement("Sprite");
					{
						xmlPrinter.PushAttribute("Name", sprite->Name.c_str());

						xmlPrinter.PushAttribute("TextureIndex", sprite->DesiredTextureIndex);

						xmlPrinter.PushAttribute("X", sprite->PackedPosition.x);
						xmlPrinter.PushAttribute("Y", sprite->PackedPosition.y);
						xmlPrinter.PushAttribute("Width", sprite->Size.x);
						xmlPrinter.PushAttribute("Height", sprite->Size.y);

						xmlPrinter.PushAttribute("OriginX", sprite->Origin.x);
						xmlPrinter.PushAttribute("OriginY", sprite->Origin.y);
					}
					xmlPrinter.CloseElement();
				}
			}
			xmlPrinter.CloseElement();

			File::WriteAllBytes("sprite_test_writeall.xml", xmlPrinter.CStr(), xmlPrinter.CStrSize() - 1);
			xmlPrinter.ClearBuffer();
#endif

			return true;
		}

		void Destroy()
		{
			sprPacker.Clear();
			TestFont.Destroy();
			SpriteRenderer->Destroy();
		}

		void Update(f64 deltaTime_milliseconds)
		{
			if (Keyboard::IsKeyTapped(SDLK_RIGHT))
			{
				displayedTextureIndex++;
			}

			if (Keyboard::IsKeyTapped(SDLK_LEFT))
			{
				displayedTextureIndex--;
			}

			if (Keyboard::IsKeyTapped(SDLK_SPACE))
			{
				drawSpriteBorders = !drawSpriteBorders;
			}

			displayedTextureIndex = MathExtensions::Clamp<i32>(displayedTextureIndex, 0, textureCount - 1);
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			Renderer::GetInstance()->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);
			const SheetTextureInfo* texStats = sprPacker.GetTextureInfo(displayedTextureIndex);

			RectangleF viewportSize = Renderer::GetInstance()->GetViewportSize();
			float yOffset = TestFont.LineHeight * 2.0f + 2.0f;

			SpriteRenderer->ResetSprite();
			SpriteRenderer->SetSpritePosition(vec2{ 0.0f, yOffset });
			SpriteRenderer->SetSpriteScale(texStats->Size);
			SpriteRenderer->PushSprite(sprTextures[displayedTextureIndex]);

			char textBuffer[128]{};
			SDL_snprintf(textBuffer, sizeof(textBuffer) - 1,
				"Texture: %d/%llu\nTexture Size: %dx%d Rectangles: %llu (Total: %llu)",
				displayedTextureIndex + 1, textureCount, texStats->RealSize.x, texStats->RealSize.x, texStats->SpriteCount, sprPacker.GetSpriteCount());

			SpriteRenderer->Font().PushString(TestFont, textBuffer, vec2(0.0f), vec2(1.0f), DefaultColors::White);
			
			SDL_snprintf(textBuffer, sizeof(textBuffer) - 1,
				"Left/Right - Change Displayed Texture\nSpace - Display Sprite Borders (toggle)");

			SpriteRenderer->Font().PushString(TestFont, textBuffer, vec2(0.0f, viewportSize.Height - yOffset), vec2(1.0f), DefaultColors::White);

			if (drawSpriteBorders)
			{
				SDL_snprintf(textBuffer, sizeof(textBuffer) - 1,
					"%dx%d", texStats->Size.x, texStats->Size.x);

				SpriteRenderer->Font().PushString(TestFont, textBuffer, vec2(0.0f, texStats->Size.y + yOffset), vec2(1.0f), DefaultColors::Red);

				SpriteRenderer->PushOutlineRect(vec2{ 0.0f, yOffset }, texStats->Size, vec2(0.0f), DefaultColors::Red);
				SpriteRenderer->PushOutlineRect(vec2{ 0.0f, yOffset }, texStats->RealSize, vec2(0.0f), DefaultColors::Purple);

				for (i32 i = 0; i < sprPacker.GetSpriteCount(); i++)
				{
					const SpriteInfo* sprite = sprPacker.GetSpriteInfo(i);
					if (!sprite->WasPacked || sprite->DesiredTextureIndex != displayedTextureIndex) { continue; }

					const ivec2& pos = sprite->PackedPosition;
					const ivec2& size = sprite->Size;
					const vec2& origin = sprite->Origin;

					// Rect outline
					SpriteRenderer->PushOutlineRect(vec2{ pos.x, pos.y + yOffset }, vec2{ size.x, size.y }, vec2(0.0f), DefaultColors::Green);

					// Rect origin points
					SpriteRenderer->PushLine(vec2{ pos.x, pos.y + origin.y + yOffset }, 0.0f, size.x, DefaultColors::Red); // X
					SpriteRenderer->PushLine(vec2{ pos.x + origin.x, pos.y + yOffset }, MathExtensions::PiOver2, size.y, DefaultColors::Red); // Y

					// Rect info
#if 0
					SDL_snprintf(textBuffer, sizeof(textBuffer) - 1,
						"%s (%02d)\n%dx%d",
						it.Name.c_str(), i, size.x, size.x);

					SpriteRenderer->Font().PushString(TestFont, textBuffer, vec2{ pos.x + 2.0f, pos.y + 2.0f + yOffset }, vec2(1.0f), DefaultColors::White);
#endif
				}
			}

			SpriteRenderer->RenderSprites(nullptr);

			Renderer::GetInstance()->SwapBuffers();
		}
	};

	SpriteTest::SpriteTest() : impl(new SpriteTest::Impl())
	{
	}

	bool SpriteTest::Initialize()
	{
		return impl->Initialize();
	}

	bool SpriteTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void SpriteTest::UnloadContent()
	{
	}

	void SpriteTest::Destroy()
	{
		impl->Destroy();
		delete impl;
	}

	void SpriteTest::Update(f64 deltaTime_milliseconds)
	{
		impl->Update(deltaTime_milliseconds);
	}

	void SpriteTest::Draw(f64 deltaTime_milliseconds)
	{
		impl->Draw(deltaTime_milliseconds);
	}

	std::string_view SpriteTest::GetStateName() const
	{
		return "[Dev] Sprite Rendering/Packing Test";
	}
}
