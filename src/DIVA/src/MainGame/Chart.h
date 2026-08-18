#pragma once
#include "Common/Types.h"
#include "TimeSpan.h"
#include "IO/Xml.h"
#include <vector>

using Starshine::TimeSpan;

namespace DIVA::MainGame
{
	enum class NoteShape : u8
	{
		Circle,
		Cross,
		Square,
		Triangle,
		Star,

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

	enum NoteTypeFlags : u8
	{
		NoteTypeFlags_None = 0,
		NoteTypeFlags_Normal = 1 << static_cast<u8>(NoteType::Normal),
		NoteTypeFlags_Double = 1 << static_cast<u8>(NoteType::Double),
		NoteTypeFlags_HoldStart = 1 << static_cast<u8>(NoteType::HoldStart),
		NoteTypeFlags_HoldEnd = 1 << static_cast<u8>(NoteType::HoldEnd),

		NoteTypeFlags_NormalAll = (NoteTypeFlags_Normal | NoteTypeFlags_Double),
		NoteTypeFlags_HoldAll = (NoteTypeFlags_HoldStart | NoteTypeFlags_HoldEnd),
		NoteTypeFlags_All = (NoteTypeFlags_NormalAll | NoteTypeFlags_HoldAll)
	};

	constexpr NoteTypeFlags NoteTypeToNoteTypeFlags(NoteType type)
	{
		return static_cast<NoteTypeFlags>(1 << static_cast<u8>(type));
	}

	constexpr Starshine::EnumStringMappingTable<NoteShape> NoteShapeStringTable
	{
		Starshine::EnumStringMapping<NoteShape>
		{ NoteShape::Circle, "Circle" },
		{ NoteShape::Cross, "Cross" },
		{ NoteShape::Square, "Square" },
		{ NoteShape::Triangle, "Triangle" },
		{ NoteShape::Star, "Star" }
	};

	constexpr Starshine::EnumStringMappingTable<NoteType> NoteTypeStringTable
	{
		Starshine::EnumStringMapping<NoteType>
		{ NoteType::Normal, "Normal" },
		{ NoteType::Double, "Double" },
		{ NoteType::HoldStart, "HoldStart" },
		{ NoteType::HoldEnd, "HoldEnd" }
	};

	struct ChartNote
	{
		TimeSpan AppearTime{};
		NoteShape Shape{};
		NoteType Type{};

		f32 X{};
		f32 Y{};

		f32 Angle{};
		f32 Frequency{};
		f32 Amplitude{};
		f32 Distance{};

		ChartNote* NextNote{};
	};

	struct NoteTimeChange
	{
		TimeSpan Time{};
		TimeSpan Value{};
	};

	struct ChanceTime
	{
		TimeSpan StartTime{};
		TimeSpan EndTime{};
	};

	class Chart
	{
	public:
		TimeSpan Duration{};
	public:
		std::vector<ChartNote> Notes;
		std::vector<NoteTimeChange> NoteTimeChanges;
		std::vector<ChanceTime> ChanceTimes;

	public:
		void ProcessNoteReferences();
		void Clear();

		void RemapToResolution(const vec2& targetResolution);

		bool LoadXml(std::string_view filePath);

		TimeSpan GetNoteTime(const TimeSpan& time);
		const ChanceTime* GetNextChanceTime(const TimeSpan& time);
	};
}
