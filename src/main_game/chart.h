#pragma once
#include "../common/types.h"
#include <vector>
#include <string>

namespace MainGame
{
	enum class NoteShape : u8
	{
		Circle,
		Cross,
		Square,
		Triangle,
		//Star,

		Count
	};

	enum class NoteType : u8
	{
		Normal,
		Double,
		HoldStart,
		HoldEnd,

		Count
	};

	constexpr DIVA::EnumStringMappingTable<NoteShape> NoteShapeStringTable
	{
		DIVA::EnumStringMapping<NoteShape>
		{ NoteShape::Circle, "Circle" },
		{ NoteShape::Cross, "Cross" },
		{ NoteShape::Square, "Square" },
		{ NoteShape::Triangle, "Triangle" }
	};

	constexpr DIVA::EnumStringMappingTable<NoteType> NoteTypeStringTable
	{
		DIVA::EnumStringMapping<NoteType>
		{ NoteType::Normal, "Normal" },
		{ NoteType::Double, "Double" },
		{ NoteType::HoldStart, "HoldStart" },
		{ NoteType::HoldEnd, "HoldEnd" }
	};

	struct ChartNote
	{
		f32 AppearTime;
		NoteShape Shape;
		NoteType Type;

		float X;
		float Y;

		float Angle;
		float Frequency;
		float Amplitude;
		float Distance;

		u32 NextNoteIndex = 0;
	};

	class Chart
	{
	public:
		std::vector<ChartNote> Notes;

	public:
		void ProcessNoteReferences();
		void Clear();
	};

	void ReadXmlChart(Chart& outChart, const char* xml, size_t size);
}
