#pragma once
#include <Common/Types.h>
#include <Common/Color.h>
#include "Graphics/AnimationSet.h"

namespace Starshine::Rendering::Render2D
{
	class SpriteRenderer;

	class AnimationSetRenderer
	{
	public:
		AnimationSetRenderer(SpriteRenderer& renderer);
		~AnimationSetRenderer() = default;

	public:
		void DrawAnimation(Graphics::AnimationSet& animSet, const Graphics::Animation& anim, f32 frame);

	private:
		SpriteRenderer& sprRenderer;
	};
}
