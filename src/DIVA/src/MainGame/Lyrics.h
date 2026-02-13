#pragma once
#include "Common/Types.h"
#include "Common/Color.h"
#include <vector>

namespace DIVA::MainGame
{
	namespace Lyrics
	{
		struct Lyric
		{
			f32 StartTime{};
			f32 EndTime{};
			Starshine::Color Color{};
			std::string Text{};
		};

		bool LoadXml(std::string_view filePath, std::vector<Lyric>& lyricsList);
	}
}
