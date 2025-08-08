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
		size_t noteCount = static_cast<size_t>(root->ChildElementCount("Note"));

		XMLElement* xmlNote = root->FirstChildElement("Note");
		for (size_t i = 0; i < noteCount; i++)
		{
			ChartNote& newNote = outChart.Notes.emplace_back();
			
			const XMLAttribute* curAttrib = xmlNote->FindAttribute("Time");
			curAttrib->QueryFloatValue(&newNote.AppearTime);

			curAttrib = xmlNote->FindAttribute("Shape");
			newNote.Shape = DIVA::EnumFromString<NoteShape>(NoteShapeStringTable, curAttrib->Value());

			curAttrib = xmlNote->FindAttribute("Type");
			newNote.Type = DIVA::EnumFromString<NoteType>(NoteTypeStringTable, curAttrib->Value());

			curAttrib = xmlNote->FindAttribute("X");
			curAttrib->QueryFloatValue(&newNote.X);

			curAttrib = xmlNote->FindAttribute("Y");
			curAttrib->QueryFloatValue(&newNote.Y);

			curAttrib = xmlNote->FindAttribute("Angle");
			curAttrib->QueryFloatValue(&newNote.Angle);

			curAttrib = xmlNote->FindAttribute("Frequency");
			curAttrib->QueryFloatValue(&newNote.Frequency);

			curAttrib = xmlNote->FindAttribute("Amplitude");
			curAttrib->QueryFloatValue(&newNote.Amplitude);

			curAttrib = xmlNote->FindAttribute("Distance");
			curAttrib->QueryFloatValue(&newNote.Distance);

			xmlNote = xmlNote->NextSiblingElement("Note");
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
};
