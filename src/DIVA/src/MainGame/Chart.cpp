#include "Chart.h"

namespace DIVA::MainGame
{
	constexpr TimeSpan DefaultNoteDuration = Starshine::TimeSpanConversion::FromSeconds(2.0);

	using namespace Starshine;

	bool Chart::LoadXml(std::string_view filePath)
	{
		Xml::Document chartDoc;
		if (!Xml::ParseFromFile(chartDoc, filePath)) { return false; }

		Xml::Element* rootElement = chartDoc.RootElement();
		float time_seconds = 0.0f;
		rootElement->QueryFloatAttribute("Duration", &time_seconds);
		Duration = Starshine::TimeSpanConversion::FromSeconds(time_seconds);

		size_t noteCount = static_cast<size_t>(rootElement->ChildElementCount("Note"));
		size_t noteTimeChangesCount = static_cast<size_t>(rootElement->ChildElementCount("SetNoteTime"));
		size_t chanceTimeCount = static_cast<size_t>(rootElement->ChildElementCount("ChanceTimeStart"));
		Notes.reserve(noteCount);
		NoteTimeChanges.reserve(noteTimeChangesCount);
		ChanceTimes.reserve(chanceTimeCount);

		Xml::Element* element = rootElement->FirstChildElement("Note");
		const Xml::Attribute* curAttrib = nullptr;
		for (size_t i = 0; i < noteCount; i++)
		{
			ChartNote& newNote = Notes.emplace_back();

			time_seconds = 0.0f;
			element->QueryFloatAttribute("Time", &time_seconds);
			newNote.AppearTime = Starshine::TimeSpanConversion::FromSeconds(time_seconds);

			curAttrib = element->FindAttribute("Shape");
			newNote.Shape = Starshine::EnumFromString<NoteShape>(NoteShapeStringTable, curAttrib->Value());

			curAttrib = element->FindAttribute("Type");
			newNote.Type = Starshine::EnumFromString<NoteType>(NoteTypeStringTable, curAttrib->Value());

			element->QueryFloatAttribute("X", &newNote.X);
			element->QueryFloatAttribute("Y", &newNote.Y);
			
			element->QueryFloatAttribute("Angle", &newNote.Angle);
			element->QueryFloatAttribute("Frequency", &newNote.Frequency);
			element->QueryFloatAttribute("Amplitude", &newNote.Amplitude);
			element->QueryFloatAttribute("Distance", &newNote.Distance);
			
			element = element->NextSiblingElement("Note");
		}

		element = rootElement->FirstChildElement("SetNoteTime");
		for (size_t i = 0; i < noteTimeChangesCount; i++)
		{
			NoteTimeChange& newTimeChange = NoteTimeChanges.emplace_back();

			time_seconds = 0.0f;
			element->QueryFloatAttribute("Time", &time_seconds);
			newTimeChange.Time = Starshine::TimeSpanConversion::FromSeconds(time_seconds);

			element->QueryFloatAttribute("Value", &time_seconds);
			newTimeChange.Value = Starshine::TimeSpanConversion::FromSeconds(time_seconds);

			element = element->NextSiblingElement("SetNoteTime");
		}

		element = rootElement->FirstChildElement("ChanceTimeStart");
		for (size_t i = 0; i < chanceTimeCount; i++)
		{
			ChanceTime chanceTime{};

			time_seconds = 0.0f;
			element->QueryFloatAttribute("Time", &time_seconds);
			chanceTime.StartTime = Starshine::TimeSpanConversion::FromSeconds(time_seconds);

			element = rootElement->FirstChildElement("ChanceTimeEnd");
			if (element != nullptr)
			{
				time_seconds = 0.0f;
				element->QueryFloatAttribute("Time", &time_seconds);
				chanceTime.EndTime = Starshine::TimeSpanConversion::FromSeconds(time_seconds);

				ChanceTimes.push_back(chanceTime);
			}
		}

		ProcessNoteReferences();
		chartDoc.Clear();

		return true;
	}

	void Chart::Clear()
	{
		Notes.clear();
		NoteTimeChanges.clear();
		ChanceTimes.clear();
	}

	void Chart::ProcessNoteReferences()
	{
		size_t i = 0;
		for (auto note = Notes.begin(); note != Notes.end(); note++)
		{
			if (note->Type == NoteType::HoldStart)
			{
				for (auto nextNote = Notes.begin() + i; nextNote != Notes.end(); nextNote++)
				{
					if (nextNote->Type == NoteType::HoldEnd && nextNote->Shape == note->Shape)
					{ 
						note->NextNote = &(*nextNote);
						break;
					}
				}
			}
			i++;
		}
	}

	TimeSpan Chart::GetNoteTime(const TimeSpan& time)
	{
		if (NoteTimeChanges.size() == 0)
		{
			return DefaultNoteDuration;
		}

		const auto* prevTimeChange = &NoteTimeChanges[0];
		for (size_t i = 0; i <= NoteTimeChanges.size(); i++)
		{
			if (i == NoteTimeChanges.size()) { return NoteTimeChanges[i - 1].Value; }
			const auto* timeChange = &NoteTimeChanges[i];

			if (timeChange->Time <= time)
			{
				prevTimeChange = timeChange;
				continue;
			}

			return prevTimeChange->Value;
		}

		return DefaultNoteDuration;
	}

	const ChanceTime* Chart::GetNextChanceTime(const TimeSpan& time)
	{
		if (ChanceTimes.empty()) { return nullptr; }

		for (auto& chanceTime : ChanceTimes)
		{
			if (chanceTime.StartTime <= time && chanceTime.EndTime >= time) { return &chanceTime; }
		}

		return nullptr;
	}
};
