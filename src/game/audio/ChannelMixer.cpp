#include "ChannelMixer.h"
#include <SDL2/SDL.h>

namespace Starshine::Audio
{
	size_t ChannelMixer::MixChannels(u32 srcChannels, i16* srcSamples, size_t srcSampleCount, i16* dstSamples, u32 dstChannels)
	{
		if (srcChannels == dstChannels)
		{
			SDL_memcpy(dstSamples, srcSamples, srcSampleCount * sizeof(i16));
			return srcSampleCount;
		}
		else if (srcChannels < dstChannels)
		{
			if (srcChannels == 1) // Mono
			{
				size_t targetIndex = 0;
				for (size_t sampleIndex = 0; sampleIndex < srcSampleCount; sampleIndex++)
				{
					for (size_t c = 0; c < dstChannels; c++)
					{
						dstSamples[targetIndex++] = srcSamples[sampleIndex];
					}
				}
				return srcSampleCount;
			}
		}

		return 0;
	}
}
