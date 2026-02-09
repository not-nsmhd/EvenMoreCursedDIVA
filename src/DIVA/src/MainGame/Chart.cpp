#include "Chart.h"

namespace DIVA::MainGame
{
	constexpr f32 DefaultNoteDuration { 60.0f / 120.0f * 4.0f };

	using namespace Starshine;

	bool Chart::LoadXml(std::string_view filePath)
	{
		Xml::Document chartDoc;
		if (!Xml::ParseFromFile(chartDoc, filePath)) { return false; }

		Xml::Element* rootElement = chartDoc.RootElement();
		rootElement->QueryFloatAttribute("Duration", &Duration);

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

			element->QueryFloatAttribute("Time", &newNote.AppearTime);

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

			element->QueryFloatAttribute("Time", &newTimeChange.Time);
			element->QueryFloatAttribute("Value", &newTimeChange.Value);

			element = element->NextSiblingElement("SetNoteTime");
		}

		element = rootElement->FirstChildElement("ChanceTimeStart");
		for (size_t i = 0; i < chanceTimeCount; i++)
		{
			ChanceTime chanceTime{};
			element->QueryFloatAttribute("Time", &chanceTime.StartTime);

			element = rootElement->FirstChildElement("ChanceTimeEnd");
			if (element != nullptr)
			{
				element->QueryFloatAttribute("Time", &chanceTime.EndTime);
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

	f32 Chart::GetNoteTime(f32 timeSeconds)
	{
		if (NoteTimeChanges.size() == 0)
		{
			return DefaultNoteDuration;
		}

		for (auto& noteTimeChange : NoteTimeChanges)
		{
			if (noteTimeChange.Time <= timeSeconds)
			{
				return noteTimeChange.Value;
			}
		}

		return DefaultNoteDuration;
	}

	const ChanceTime* Chart::GetNextChanceTime(f32 timeSeconds)
	{
		if (ChanceTimes.empty()) { return nullptr; }

		for (auto& chanceTime : ChanceTimes)
		{
			if (chanceTime.StartTime <= timeSeconds && chanceTime.EndTime >= timeSeconds) { return &chanceTime; }
		}

		return nullptr;
	}
};
