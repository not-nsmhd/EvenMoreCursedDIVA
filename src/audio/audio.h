#pragma once
#include <SDL2/SDL.h>
#include <FAudio.h>
#include "common/int_types.h"

namespace Audio
{
	struct AudioVoice;

	class Audio
	{
		const u32 MAX_VOICES_SFX = 48;
		const u32 MAX_VOICES_STREAMING = 4;

	protected:
		Audio();
		static Audio* instance;

	private:
		FAudio* faudioBackend;
		FAudioMasteringVoice* masteringVoice;

		AudioVoice* voices;
	};
}
