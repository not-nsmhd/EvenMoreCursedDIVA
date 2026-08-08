#include "AnimationSetRenderer.h"
#include "SpriteRenderer.h"

using namespace Starshine::Graphics;

namespace Starshine::Rendering::Render2D
{
	AnimationSetRenderer::AnimationSetRenderer(SpriteRenderer& renderer) : sprRenderer(renderer)
	{
	}

	void AnimationSetRenderer::DrawAnimation(Graphics::AnimationSet& animSet, const Graphics::Animation& anim, f32 frame)
	{
		BlendMode prevBlendMode{};
		for (const auto& layer : anim.Layers)
		{
			if (frame < layer.StartTime || frame > layer.EndTime || !layer.Visible)
				continue;

			const Transform2D transform = layer.GetTransform(frame);
			const SpriteDefinition* spriteDef = layer.SpriteDefinition;
			const vec2& spriteSize = spriteDef->Size;
			const vec2& spriteLayerSize = spriteSize * transform.Scale;

			if (prevBlendMode != layer.BlendMode)
			{
				sprRenderer.RenderSprites(nullptr);
				sprRenderer.SetBlendMode(layer.BlendMode);
			}

			i32 texIndex{};

			if (spriteDef->RealSprite != nullptr)
			{
				sprRenderer.SpriteSheet().SetSpriteState(*animSet.GetSpriteSheet(), *spriteDef->RealSprite, vec2{}, &texIndex);
				sprRenderer.SetSpriteOrigin(transform.Origin * spriteLayerSize);
				sprRenderer.SetSpritePosition(transform.Position);
				sprRenderer.SetSpriteSize(transform.Scale * spriteSize);
				sprRenderer.SetSpriteRotation(MathExtensions::ToRadians(transform.Rotation));
				sprRenderer.SetSpriteColor(transform.Color);
				sprRenderer.PushSprite(animSet.GetSpriteSheet()->GetTexture(texIndex));
			}

			prevBlendMode = layer.BlendMode;
		}

		sprRenderer.RenderSprites(nullptr);
	}
}
