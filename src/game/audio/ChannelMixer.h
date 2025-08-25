#pragma once
#include "common/types.h"

namespace Starshine::Audio
{
	class ChannelMixer : NonCopyable
	{
	public:
		size_t MixChannels(u32 srcChannels, i16* srcSamples, size_t srcSampleCount, i16* dstSamples, u32 dstChannels);
	};
}
