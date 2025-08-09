#include "chart.h"
#include <tinyxml2.h>

namespace MainGame
{
	using namespace tinyxml2;

	void ReadXmlChart(Chart& outChart, const char* xml, size_t size)
	{
		XMLDocument xmlDoc;
		xmlDoc.Parse(xml, size);

		XMLElement* root = xmlDoc.RootElement();

		const XMLAttribute* curAttrib = root->FindAttribute("Duration");
		curAttrib->QueryFloatValue(&outChart.Duration);

		size_t noteCount = static_cast<size_t>(root->ChildElementCount("Note"));
		size_t noteTimeChangesCount = static_cast<size_t>(root->ChildElementCount("SetNoteTime"));
		outChart.Notes.reserve(noteCount);
		outChart.NoteTimeChanges.reserve(noteTimeChangesCount);

		XMLElement* xmlElement = root->FirstChildElement("Note");
		for (size_t i = 0; i < noteCount; i++)
		{
			ChartNote& newNote = outChart.Notes.emplace_back();
			
			curAttrib = xmlElement->FindAttribute("Time");
			curAttrib->QueryFloatValue(&newNote.AppearTime);

			curAttrib = xmlElement->FindAttribute("Shape");
			newNote.Shape = DIVA::EnumFromString<NoteShape>(NoteShapeStringTable, curAttrib->Value());

			curAttrib = xmlElement->FindAttribute("Type");
			newNote.Type = DIVA::EnumFromString<NoteType>(NoteTypeStringTable, curAttrib->Value());

			curAttrib = xmlElement->FindAttribute("X");
			curAttrib->QueryFloatValue(&newNote.X);

			curAttrib = xmlElement->FindAttribute("Y");
			curAttrib->QueryFloatValue(&newNote.Y);

			curAttrib = xmlElement->FindAttribute("Angle");
			curAttrib->QueryFloatValue(&newNote.Angle);

			curAttrib = xmlElement->FindAttribute("Frequency");
			curAttrib->QueryFloatValue(&newNote.Frequency);

			curAttrib = xmlElement->FindAttribute("Amplitude");
			curAttrib->QueryFloatValue(&newNote.Amplitude);

			curAttrib = xmlElement->FindAttribute("Distance");
			curAttrib->QueryFloatValue(&newNote.Distance);

			xmlElement = xmlElement->NextSiblingElement("Note");
		}

		xmlElement = root->FirstChildElement("SetNoteTime");
		for (size_t i = 0; i < noteTimeChangesCount; i++)
		{
			NoteTimeChange& newTimeChange = outChart.NoteTimeChanges.emplace_back();

			curAttrib = xmlElement->FindAttribute("Time");
			curAttrib->QueryFloatValue(&newTimeChange.Time);

			curAttrib = xmlElement->FindAttribute("Value");
			curAttrib->QueryFloatValue(&newTimeChange.Value);
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
			return 2.0f;
		}

		for (auto& noteTimeChange : NoteTimeChanges)
		{
			if (noteTimeChange.Time <= timeSeconds)
			{
				return noteTimeChange.Value;
			}
		}
	}
};
