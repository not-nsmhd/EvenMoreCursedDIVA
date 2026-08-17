#pragma once
#include <Common/Types.h>
#include <Graphics/AnimationSet.h>

namespace Starshine
{
	class DebugKeyframeList : NonCopyable
	{
	public:
		DebugKeyframeList() = default;
		~DebugKeyframeList() = default;

	public:
		void SetAnimation(Graphics::AnimationSet* animSet, Graphics::Animation* anim);
		void OnGUI();

	public:
		bool DrawWindow{};

	private:
		Graphics::AnimationSet* currentAnimSet{};
		Graphics::Animation* currentAnim{};
		Graphics::Layer* layerToDisplay{};
	};
}
