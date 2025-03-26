#include "event.h"

namespace MainGame
{
	const char *g_EventTypeNames[] =
	{
		"SetBPM",
		"SongEnd",
		"PlayMusic"
	};

	const std::unordered_map<std::string, ChartEventType> g_ChartEventTypeConversionTable =
	{
		{ "SetBPM", ChartEventType::EVENT_SET_BPM },
		{ "SongEnd", ChartEventType::EVENT_SONG_END },
		{ "PlayMusic", ChartEventType::EVENT_PLAY_MUSIC }
	};
}
