#pragma once
#include "Common/Types.h"
#include "Common/TimeSpan.h"
#include "IO/Xml.h"
#include <vector>

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
		Starshine::TimeSpan AppearTime{};
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
		Starshine::TimeSpan Time{};
		Starshine::TimeSpan Value{};
	};

	struct ChanceTime
	{
		Starshine::TimeSpan StartTime{};
		Starshine::TimeSpan EndTime{};
	};

	struct TechnicalZone
	{
		Starshine::TimeSpan StartTime{};
		Starshine::TimeSpan EndTime{};
		i32 NoteCount{};
	};

	class Chart
	{
	public:
		Starshine::TimeSpan Duration{};
	public:
		std::vector<ChartNote> Notes;
		std::vector<NoteTimeChange> NoteTimeChanges;
		std::vector<ChanceTime> ChanceTimes;
		std::vector<TechnicalZone> TechnicalZones;

	public:
		void ProcessNoteReferences();
		void Clear();

		void RemapToResolution(const vec2& targetResolution);

		bool LoadXml(std::string_view filePath);

		Starshine::TimeSpan GetNoteTime(const Starshine::TimeSpan& time);
		const ChanceTime* GetNextChanceTime(const Starshine::TimeSpan& time);
		const TechnicalZone* GetNextTechnicalZone(const Starshine::TimeSpan& time);
	};
}
