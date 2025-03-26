#pragma once
#include <unordered_map>
#include <string>

namespace MainGame
{
	enum class ChartEventType
	{
		EVENT_NONE = -1,

		EVENT_SET_BPM,
		EVENT_SONG_END,
		EVENT_PLAY_MUSIC
	};

	extern const char* g_EventTypeNames[];
	extern const std::unordered_map<std::string, ChartEventType> g_ChartEventTypeConversionTable;

	struct ChartEvent
	{
		float ExecutionTime;
		ChartEventType Type;
	};

	struct SetBPMEvent : public ChartEvent
	{
		float BPM;
		int BeatsPerBar;
	};
}
