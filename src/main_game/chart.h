#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <glm/vec2.hpp>
#include "io/filesystem.h"

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
		NOTE_STAR
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

	const std::string g_NoteShapeNames[] = 
	{
		"Triangle",
		"Circle",
		"Cross",
		"Square",
		"Star"
	};

	const std::unordered_map<std::string, NoteShape> g_NoteShapeConversionTable =
	{
		{ "Triangle", 	NoteShape::NOTE_TRIANGLE },
		{ "Circle", 	NoteShape::NOTE_CIRCLE },
		{ "Cross", 		NoteShape::NOTE_CROSS },
		{ "Square", 	NoteShape::NOTE_SQUARE },
		{ "Star", 		NoteShape::NOTE_STAR }
	};

	class Chart
	{
	public:
		Chart();
		~Chart();

		std::vector<ChartNote> Notes;

		void Clear();

		bool ReadFromXml(const char* xml, size_t size);
		bool LoadFromXml(const std::filesystem::path& path);
		std::string WriteToXml();
	};
}
