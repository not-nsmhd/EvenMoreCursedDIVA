#include "audio.h"

namespace Audio
{
	struct AudioVoice
	{
		FAudioVoice* faudioVoice = nullptr;
		u32 queuedBuffers = 0;
		bool active = false;
	};

	
}
