#pragma once
#include <filesystem>
#include "sound_effect.h"

namespace Audio
{
	namespace Helpers
	{
		bool LoadSoundEffect(SoundEffect* output, const std::filesystem::path& path);
	}
}
