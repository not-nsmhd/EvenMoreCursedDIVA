#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <glm/vec2.hpp>
#include "io/filesystem.h"
#include "event.h"

using glm::vec2;

namespace MainGame
{
	enum class NoteShape
	{
		NOTE_NONE = -1,
		NOTE_TRIANGLE,
		NOTE_CIRCLE,
		NOTE_CROSS,
		NOTE_SQUARE,
		NOTE_STAR,

		NOTE_SHAPE_COUNT
	};

	struct ChartNote
	{
		float AppearTime;

		NoteShape Shape;
		int ReferenceIndex;

		vec2 Position;
		float Angle;
		float Frequency;
		float Amplitude;
		float Distance;
	};

	extern const char* g_NoteShapeNames[];
	extern const std::unordered_map<std::string, NoteShape> g_NoteShapeConversionTable;

	class Chart
	{
	public:
		Chart();
		~Chart();

		std::vector<ChartNote> Notes;
		std::vector<ChartEvent*> Events;

		void Clear();

		bool ReadFromXml(const char* xml, size_t size);
		bool LoadFromXml(const std::filesystem::path& path);
		std::string WriteToXml();
	};
}
