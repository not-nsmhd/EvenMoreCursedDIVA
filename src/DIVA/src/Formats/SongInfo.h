#pragma once
#include "Common/Types.h"

namespace DIVA::Formats
{
	enum class ChartDifficulty : i32
	{
		Easy,
		Normal,
		Hard,
		Extreme,

		Count
	};

	constexpr std::array<const char*, Starshine::EnumCount<ChartDifficulty>()> ChartDifficultyNames
	{
		"Easy",
		"Normal",
		"Hard",
		"Extreme"
	};

	struct SongInfo
	{
		std::string Name;

		std::array<std::string, Starshine::EnumCount<ChartDifficulty>()> ChartFilePaths;
		std::string MusicFilePath;

	public:
		bool ParseFromFile(std::string_view filePath);
	};
}
