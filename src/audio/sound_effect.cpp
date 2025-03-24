#include "sound_effect.h"

namespace Audio
{
	void SoundEffect::Create(int channels, int rate, size_t samples)
	{
		if (samples > 0)
		{
			pcmData = new u16[samples];
			pcmDataSize = samples;

			audioBuffer.AudioBytes = samples;
			audioBuffer.pAudioData = (const uint8_t*)pcmData;
			audioBuffer.Flags = FAUDIO_END_OF_STREAM;
		}
	}
	
	void SoundEffect::Destroy()
	{
		if (pcmDataSize > 0)
		{
			delete[] pcmData;
			pcmDataSize = 0;
		}
	}
	
	u16* SoundEffect::GetPCMData()
	{
		if (pcmDataSize > 0)
		{
			return pcmData;
		}
		return nullptr;
	}
	
	FAudioBuffer* SoundEffect::GetBuffer()
	{
		if (pcmDataSize > 0)
		{
			return &audioBuffer;
		}
		return nullptr;
	}
}
