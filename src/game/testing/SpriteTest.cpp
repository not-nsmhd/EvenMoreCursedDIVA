#include "SpriteTest.h"
#include <Common/Types.h>
#include <Common/MathExt.h>
#include "gfx/Renderer.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "gfx/Render2D/SpriteSheet.h"

#include <GFX/RectanglePacker.h>
#include <IO/Path/Directory.h>
#include <IO/Xml.h>
#include <vector>
#include <stb_image.h>

#include "util/logging.h"

namespace Starshine::Testing
{
	using namespace Starshine::GFX;
	using namespace Starshine::GFX::Render2D;
	using namespace Starshine::IO;

	struct SpriteTest::Impl
	{
		SpriteRenderer* SpriteRenderer = nullptr;
		Font TestFont;

		struct SpritePackingOptions
		{
			std::string ImagePath;
			std::string Name;
			ivec2 Size{};
			vec2 Origin{};

			i32 DesiredTextureIndex{};

			i32 PackedRectangleIndex{ -1 };
		};

		std::vector<SpritePackingOptions> spritesToPack;
		RectanglePacker packer;

		bool Initialize()
		{
			packer.Initialize();
			return true;
		}

		bool LoadContent()
		{
			SpriteRenderer = new Render2D::SpriteRenderer();
			TestFont.ReadBMFont("diva/fonts/debug.fnt");

			Directory::IterateFiles("diva/sprites/devtest", 
				[&](std::string_view filePath)
				{
					std::string_view fileExt = Path::GetExtension(filePath);
					std::string_view fileName = Path::GetFileName(filePath, false);
					if (fileExt == ".png")
					{
						SpritePackingOptions spriteOptions{};
						int channels = 0;
						if (stbi_info(filePath.data(), &spriteOptions.Size.x, &spriteOptions.Size.y, &channels) != 0)
						{
							spriteOptions.Origin = vec2(
								static_cast<float>(spriteOptions.Size.x) / 2.0f,
								static_cast<float>(spriteOptions.Size.y) / 2.0f);

							spriteOptions.ImagePath = filePath;
							spriteOptions.Name = fileName;

							spritesToPack.push_back(spriteOptions);
						}
					}
					else if (fileExt == ".xml")
					{
						SpritePackingOptions* linkedSprite = nullptr;
						for (auto& it : spritesToPack)
						{
							if (it.Name == fileName) { linkedSprite = &it; break; }
						}

						if (linkedSprite == nullptr) { return; }

						Xml::Document doc;
						if (Xml::ParseFromFile(doc, filePath) != true) { return; }

						const Xml::Element* rootElement = Xml::GetRootElement(doc);
						if (rootElement != nullptr)
						{
							const Xml::Element* optionsElement = Xml::FindElement(rootElement, "SpritePackOptions");
							if (optionsElement != nullptr)
							{
								// TODO: Source shifting
								const Xml::Attribute* originX_Attrib = Xml::FindAttribute(optionsElement, "OriginX");
								const Xml::Attribute* originY_Attrib = Xml::FindAttribute(optionsElement, "OriginY");
								const Xml::Attribute* texAttrib = Xml::FindAttribute(optionsElement, "DesiredTextureIndex");

								if (originX_Attrib != nullptr)
								{
									f32 originX = 0.0f;
									if (originX_Attrib->QueryFloatValue(&originX) == tinyxml2::XMLError::XML_SUCCESS)
									{
										linkedSprite->Origin.x = originX;
									}
								}

								if (originY_Attrib != nullptr)
								{
									f32 originY = 0.0f;
									if (originY_Attrib->QueryFloatValue(&originY) == tinyxml2::XMLError::XML_SUCCESS)
									{
										linkedSprite->Origin.y = originY;
									}
								}

								if (texAttrib != nullptr)
								{
									i32 texIndex = 0;
									if (texAttrib->QueryIntValue(&texIndex) == tinyxml2::XMLError::XML_SUCCESS)
									{
										linkedSprite->DesiredTextureIndex = texIndex;
									}
								}
							}
						}

						doc.Clear();
					}
				});

			for (auto& it : spritesToPack)
			{
				if (it.DesiredTextureIndex == 0)
				{
					it.PackedRectangleIndex = packer.TryPack(it.Size);
				}
			}

			return true;
		}

		void Destroy()
		{
			packer.Clear();
			TestFont.Destroy();
			SpriteRenderer->Destroy();
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			Renderer::GetInstance()->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);

			ivec2 rectPackerArea = packer.GetRealAreaSize();
			char textBuffer[128]{};

			SpriteRenderer->PushOutlineRect(vec2{ 0.0f, 0.0f }, vec2(rectPackerArea), vec2(0.0f), DefaultColors::Black);

			SDL_snprintf(textBuffer, sizeof(textBuffer) - 1,
				"Area Size: %dx%d\nRectangles: %llu",
				rectPackerArea.x, rectPackerArea.y, packer.GetRectangleCount());

			SpriteRenderer->Font().PushString(TestFont, textBuffer, vec2{ 0.0f, rectPackerArea.y + 2.0f }, vec2(1.0f), DefaultColors::White);

			for (const auto& it : spritesToPack)
			{
				if (it.PackedRectangleIndex == -1) { continue; }

				const Rectangle& rect = packer.GetRectangle(static_cast<i32>(it.PackedRectangleIndex));

				// Rect outline
				SpriteRenderer->PushOutlineRect(vec2{ rect.X, rect.Y }, vec2{ rect.Width, rect.Height }, vec2(0.0f), DefaultColors::Green);

				// Rect origin points
				SpriteRenderer->PushLine(vec2{ rect.X, rect.Y + it.Origin.y }, 0.0f, rect.Width, DefaultColors::Red); // X
				SpriteRenderer->PushLine(vec2{ rect.X + it.Origin.x, rect.Y }, MathExtensions::PiOver2, rect.Height, DefaultColors::Red); // Y

				// Rect info
				SDL_snprintf(textBuffer, sizeof(textBuffer) - 1,
					"%s (%02d)\n%dx%d",
					it.Name.c_str(), it.PackedRectangleIndex, rect.Width, rect.Height);

				SpriteRenderer->Font().PushString(TestFont, textBuffer, vec2{ rect.X + 2.0f, rect.Y + 2.0f }, vec2(1.0f), DefaultColors::White);
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
