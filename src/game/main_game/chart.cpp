#include "chart.h"
#include <tinyxml2.h>

namespace DIVA::MainGame
{
	constexpr f32 DefaultNoteDuration { 60.0f / 120.0f * 4.0f };

	using namespace tinyxml2;

	void ReadXmlChart(Chart& outChart, const char* xml, size_t size)
	{
		XMLDocument xmlDoc;
		xmlDoc.Parse(xml, size);

		XMLElement* root = xmlDoc.RootElement();
		root->QueryFloatAttribute("Duration", &outChart.Duration);

		size_t noteCount = static_cast<size_t>(root->ChildElementCount("Note"));
		size_t noteTimeChangesCount = static_cast<size_t>(root->ChildElementCount("SetNoteTime"));
		outChart.Notes.reserve(noteCount);
		outChart.NoteTimeChanges.reserve(noteTimeChangesCount);

		XMLElement* xmlElement = root->FirstChildElement("Note");
		const XMLAttribute* curAttrib = nullptr;
		for (size_t i = 0; i < noteCount; i++)
		{
			ChartNote& newNote = outChart.Notes.emplace_back();
			
			xmlElement->QueryFloatAttribute("Time", &newNote.AppearTime);

			curAttrib = xmlElement->FindAttribute("Shape");
			newNote.Shape = Starshine::EnumFromString<NoteShape>(NoteShapeStringTable, curAttrib->Value());

			curAttrib = xmlElement->FindAttribute("Type");
			newNote.Type = Starshine::EnumFromString<NoteType>(NoteTypeStringTable, curAttrib->Value());

			xmlElement->QueryFloatAttribute("X", &newNote.X);
			xmlElement->QueryFloatAttribute("Y", &newNote.Y);

			xmlElement->QueryFloatAttribute("Angle", &newNote.Angle);
			xmlElement->QueryFloatAttribute("Frequency", &newNote.Frequency);
			xmlElement->QueryFloatAttribute("Amplitude", &newNote.Amplitude);
			xmlElement->QueryFloatAttribute("Distance", &newNote.Distance);

			xmlElement = xmlElement->NextSiblingElement("Note");
		}

		xmlElement = root->FirstChildElement("SetNoteTime");
		for (size_t i = 0; i < noteTimeChangesCount; i++)
		{
			NoteTimeChange& newTimeChange = outChart.NoteTimeChanges.emplace_back();

			xmlElement->QueryFloatAttribute("Time", &newTimeChange.Time);
			xmlElement->QueryFloatAttribute("Value", &newTimeChange.Value);
		}

		outChart.ProcessNoteReferences();
	}

	void Chart::Clear()
	{
		Notes.clear();
	}

	void Chart::ProcessNoteReferences()
	{
		for (size_t i = 0; i < Notes.size(); i++)
		{
			ChartNote& note = Notes[i];
			if (note.Type == NoteType::HoldStart)
			{
				for (size_t j = i; j < Notes.size(); j++)
				{
					ChartNote& nextNote = Notes[j];
					if (nextNote.Type == NoteType::HoldEnd && nextNote.Shape == note.Shape)
					{
						note.NextNoteIndex = static_cast<u32>(j);
						break;
					}
				}
			}
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
};
