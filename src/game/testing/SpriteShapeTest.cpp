#include "SpriteShapeTest.h"
#include <Common/Types.h>
#include <Common/MathExt.h>
#include "gfx/Renderer.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "gfx/Render2D/SpriteSheet.h"
#include "input/Keyboard.h"

#include <GFX/SpritePacker.h>

#include "util/logging.h"

namespace Starshine::Testing
{
	using namespace Starshine::GFX;
	using namespace Starshine::GFX::Render2D;
	using namespace Starshine::Input;

	struct SpriteShapeTest::Impl
	{
		static constexpr size_t TrailSegmentCount = 48;
		static constexpr f32 TrailMaxProgress = 1.25f;
		static constexpr f32 TrailSegmentStep = TrailMaxProgress / static_cast<f32>(TrailSegmentCount);

		std::array<vec2, TrailSegmentCount> TrailSegments{};

		SpriteRenderer* SpriteRenderer = nullptr;
		Font TestFont;

		SpritePacker sprPacker;

		SpriteSheet sprSheet;
		const Sprite* iconSprite{};
		const Sprite* trailSprite{};

		f32 trailScroll = 0.0f;
		f32 noteProgress = 0.0f;

		bool Initialize()
		{
			sprPacker.Initialize();

			f32 f = 1.0f;
			for (size_t i = 0; i < TrailSegmentCount; i++)
			{
				TrailSegments[i] = MathExtensions::GetSinePoint(f, vec2(640.0f, 360.0f), 90.0f, 2.0f, 500.0f, 1200.0f);
				f -= TrailSegmentStep;
			}

			return true;
		}

		bool LoadContent()
		{
			SpriteRenderer = new Render2D::SpriteRenderer();
			TestFont.ReadBMFont("diva/fonts/debug.fnt");

			sprPacker.AddFromDirectory("diva/sprites/devtest3");
			sprPacker.Pack();
			
			sprSheet.CreateFromSpritePacker(sprPacker);
			sprPacker.Clear();

			iconSprite = &sprSheet.GetSprite("Icon_Star");
			trailSprite = &sprSheet.GetSprite("Trail_CT");

			return true;
		}

		void Destroy()
		{
			sprSheet.Destroy();
			sprPacker.Clear();
			TestFont.Destroy();
			SpriteRenderer->Destroy();
		}

		void Update(f64 deltaTime_milliseconds)
		{
			trailScroll += deltaTime_milliseconds / 50.0f;
			trailScroll = std::fmodf(trailScroll, trailSprite->SourceRectangle.Width);

			noteProgress += deltaTime_milliseconds / 5000.0f;
			noteProgress = std::fmodf(noteProgress, 1.0f);
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			Renderer::GetInstance()->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);

#if 0
			constexpr size_t segmentCount = 48;
			constexpr size_t verticesPerSegment = 2;
			std::array<vec2, segmentCount> trailSegments;
			std::array<SpriteVertex, segmentCount * verticesPerSegment> trailVertices;

			constexpr float step = 1.0f / 48.0f;
			float percentage = 0.0f;

			for (size_t i = 0; i < trailSegments.size(); i++)
			{
				trailSegments[i] = MathExtensions::GetSinePoint(percentage, vec2(640.0f, 360.0f), 0.0f, 2.0f, 83.0f, 200.0f);
				percentage += step;
			}

			const auto getNormal = [](vec2 v) { return vec2(v.y, -v.x); };
			Texture* trailTex = sprSheet.GetTexture(trailSprite->TextureIndex);
			f32 trailTexWidth = static_cast<f32>(trailTex->GetWidth());
			f32 trailTexHeight = static_cast<f32>(trailTex->GetHeight());

			f32 startOffset = 0.0f;
			f32 segmentScrollOffset = glm::distance(trailSegments[0], trailSegments[1]) / (trailSprite->SourceRectangle.Width * 0.05f);

			static constexpr std::array<u8, segmentCount> trailAlphaValues
			{
				0, 56, 76, 90, 100, 108, 114, 119, 122, 125, 126, 127,
				127, 127, 126, 125, 124, 122, 120, 117, 114, 111, 108,
				105, 101, 98, 94, 90, 86, 82, 78, 74, 69, 65, 61, 56,
				52, 47, 43, 39, 34, 30, 25, 21, 17, 12, 0
			};

			for (size_t i = 0, v = 0; i < trailSegments.size(); i++, v += 2)
			{
				const auto normal = (i < 1) ? glm::normalize(getNormal(trailSegments[i + 1] - trailSegments[i])) :
					(i >= segmentCount - 1) ? glm::normalize(getNormal(trailSegments[i] - trailSegments[i - 1])) :
					glm::normalize(getNormal(trailSegments[i] - trailSegments[i - 1]) + getNormal(trailSegments[i + 1] - trailSegments[i]));

				trailVertices[v + 0].Position = trailSegments[i] + normal * trailSprite->SourceRectangle.Height * 0.5f;
				trailVertices[v + 1].Position = trailSegments[i] - normal * trailSprite->SourceRectangle.Height * 0.5f;

				trailVertices[v + 0].Color = Color(255, 255, 255, trailAlphaValues[i]);
				trailVertices[v + 1].Color = Color(255, 255, 255, trailAlphaValues[i]);

				trailVertices[v + 0].TexCoord = vec2(
					(trailSprite->SourceRectangle.X + startOffset + trailScroll) / trailTexWidth,
					trailSprite->SourceRectangle.Y / trailTexHeight);

				trailVertices[v + 1].TexCoord = vec2(
					(trailSprite->SourceRectangle.X + startOffset + segmentScrollOffset + trailScroll) / trailTexWidth,
					(trailSprite->SourceRectangle.Y + trailSprite->SourceRectangle.Height) / trailTexHeight);

				startOffset += segmentScrollOffset;
			}

			SpriteRenderer->PushShape(trailVertices.data(), trailVertices.size(), PrimitiveType::TriangleStrip, trailTex);
#endif

			f32 segmentRefValue = 0.0f;
			std::array<SpriteVertex, TrailSegmentCount> trailVertices;
			for (size_t i = 0; i < trailVertices.size(); i++)
			{
				trailVertices[i].Position = TrailSegments[i];
				trailVertices[i].Color = (noteProgress >= segmentRefValue) ? DefaultColors::Yellow : DefaultColors::Red;

				segmentRefValue += TrailSegmentStep;
			}

			SpriteRenderer->PushShape(trailVertices.data(), trailVertices.size(), PrimitiveType::LineStrip, nullptr);

			segmentRefValue = 0.0f;
			for (size_t i = 0; i < trailVertices.size(); i++)
			{
				trailVertices[i].Position = TrailSegments[i];
				trailVertices[i].Color = (noteProgress >= segmentRefValue) ? DefaultColors::Yellow : DefaultColors::Red;

				segmentRefValue += TrailSegmentStep;
			}

			vec2 notePosition = MathExtensions::GetSinePoint(1.0f - noteProgress, vec2(640.0f, 360.0f), 90.0f, 2.0f, 500.0f, 1200.0f);
			//SpriteRenderer->SpriteSheet().PushSprite(sprSheet, *iconSprite, notePosition, vec2(1.0f), DefaultColors::White);

			char debugText[128] = {};
			size_t pos = SDL_snprintf(debugText, sizeof(debugText) - 1, "Global Scroll Offset: %.3f", trailScroll);
			pos += SDL_snprintf(debugText + pos, sizeof(debugText) - 1, "\nNote Progress: %.3f", noteProgress);
			SpriteRenderer->Font().PushString(TestFont, debugText, vec2(0.0f), vec2(1.0f), DefaultColors::White);

			SpriteRenderer->RenderSprites(nullptr);
			Renderer::GetInstance()->SwapBuffers();
		}
	};

	SpriteShapeTest::SpriteShapeTest() : impl(new SpriteShapeTest::Impl())
	{
	}

	bool SpriteShapeTest::Initialize()
	{
		return impl->Initialize();
	}

	bool SpriteShapeTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void SpriteShapeTest::UnloadContent()
	{
	}

	void SpriteShapeTest::Destroy()
	{
		impl->Destroy();
		delete impl;
	}

	void SpriteShapeTest::Update(f64 deltaTime_milliseconds)
	{
		impl->Update(deltaTime_milliseconds);
	}

	void SpriteShapeTest::Draw(f64 deltaTime_milliseconds)
	{
		impl->Draw(deltaTime_milliseconds);
	}

	std::string_view SpriteShapeTest::GetStateName() const
	{
		return "[Dev] Sprite Rendering/Packing Test";
	}
}
