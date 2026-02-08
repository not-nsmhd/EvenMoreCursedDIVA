#pragma once
#include <Common/Types.h>
#include <vector>
#include "Formats/SongInfo.h"

namespace DIVA
{
	struct GameContext : NonCopyable
	{
		static std::vector<Formats::SongInfo> SongList;
	};
}
