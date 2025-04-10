#pragma once
#include <FAudio.h>
#include "../common/int_types.h"

namespace Audio
{
	class SoundEffect
	{
	public:
		SoundEffect() {};

		float Volume = 1.0f;

		void Create(int channels, int rate, size_t samples);
		void Destroy();

		u16* GetPCMData();
		FAudioBuffer* GetBuffer();

	private:
		u16* pcmData = nullptr;
		size_t pcmDataSize = 0;

		FAudioBuffer audioBuffer = {};
	};
}
