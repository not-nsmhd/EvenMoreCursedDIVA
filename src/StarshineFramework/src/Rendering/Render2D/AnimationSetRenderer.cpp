#include "AnimationSetRenderer.h"
#include "SpriteRenderer.h"

using namespace Starshine::Graphics;

namespace Starshine::Rendering::Render2D
{
	AnimationSetRenderer::AnimationSetRenderer(SpriteRenderer& renderer) : sprRenderer(renderer)
	{
	}

	void AnimationSetRenderer::PushAnimation(Graphics::AnimationSet* animSet, const Graphics::Animation* anim, f32 frame, const vec2& position, const vec2& scale)
	{
		if (animSet == nullptr || anim == nullptr)
			return;

		BlendMode prevBlendMode{};
		for (const auto& layer : anim->Layers)
		{
			if (frame < layer.StartTime || frame > layer.EndTime || !layer.Visible)
				continue;

			const Transform2D transform = layer.GetTransform(frame);
			const SpriteDefinition* spriteDef = layer.SpriteDefinition;
			const vec2& spriteSize = spriteDef->Size * scale;
			const vec2& spriteLayerSize = spriteSize * transform.Scale;

			if (prevBlendMode != layer.BlendMode)
			{
				vec2 basePos{};
				vec2 baseScale{};

				sprRenderer.GetBasePositionAndScale(basePos, baseScale);
				sprRenderer.RenderSprites(nullptr);
				sprRenderer.SetBasePositionAndScale(basePos, baseScale);
				sprRenderer.SetBlendMode(layer.BlendMode);
			}

			i32 texIndex{};

			if (spriteDef->RealSprite != nullptr)
				sprRenderer.SpriteSheet().SetSpriteState(*animSet->GetSpriteSheet(), *spriteDef->RealSprite, vec2{}, &texIndex);

			sprRenderer.SetSpriteOrigin(transform.Origin * spriteLayerSize);
			sprRenderer.SetSpritePosition(transform.Position + position);
			sprRenderer.SetSpriteSize(transform.Scale * spriteSize);
			sprRenderer.SetSpriteRotation(MathExtensions::ToRadians(transform.Rotation));
			sprRenderer.SetSpriteColor(transform.Color);

			if (spriteDef->RealSprite != nullptr)
				sprRenderer.PushSprite(animSet->GetSpriteSheet()->GetTexture(texIndex));
			else
				sprRenderer.PushSprite(nullptr);

			prevBlendMode = layer.BlendMode;
		}
	}

	void AnimationSetRenderer::PushAnimation(Graphics::AnimationSet* animSet, const Graphics::Animation* anim, f32 frame)
	{
		PushAnimation(animSet, anim, frame, vec2(0.0f), vec2(1.0f));
	}
}
