#include "chart.h"
#include <fstream>
#include <tinyxml2.h>
#include <SDL2/SDL.h>

using std::fstream;
using std::ios;
using std::vector;
using std::string;
using namespace tinyxml2;

using fsPath = std::filesystem::path;

namespace MainGame
{
	const char* g_NoteShapeNames[] = 
	{
		"Triangle",
		"Circle",
		"Cross",
		"Square",
		"Star"
	};

	const char* g_NoteTypeNames[] = 
	{
		"Normal",
		"Double",
		"HoldStart",
		"HoldEnd"
	};

	const std::unordered_map<string, NoteShape> g_NoteShapeConversionTable =
	{
		{ "Triangle", 	NoteShape::NOTE_TRIANGLE },
		{ "Circle", 	NoteShape::NOTE_CIRCLE },
		{ "Cross", 		NoteShape::NOTE_CROSS },
		{ "Square", 	NoteShape::NOTE_SQUARE },
		{ "Star", 		NoteShape::NOTE_STAR }
	};

	const std::unordered_map<std::string, NoteType> g_NoteTypeConversionTable = 
	{
		{ "Normal",		NoteType::TYPE_NORMAL },	
		{ "Double",		NoteType::TYPE_DOUBLE },	
		{ "HoldStart",	NoteType::TYPE_HOLD_START },
		{ "HoldEnd",	NoteType::TYPE_HOLD_END }	
	};

	Chart::Chart()
	{
	}
	
	Chart::~Chart()
	{
		Clear();
	}
	
	void Chart::Clear()
	{
		Notes.clear();

		for (vector<ChartEvent*>::iterator it = Events.begin(); it != Events.end(); it++)
		{
			delete *it; // lol object slicing
		}

		Events.clear();
	}
	
	bool Chart::ReadFromXml(const char* xml, size_t size)
	{
		XMLDocument chartDoc;
		chartDoc.Parse(xml, size + 1);

		XMLElement* rootNode = chartDoc.FirstChildElement("Chart");

		const XMLAttribute* elementAttr;
		for (XMLElement* element = rootNode->FirstChildElement(); element; element = element->NextSiblingElement())
		{
			if (SDL_strncmp(element->Name(), "Note", 5) == 0)
			{
				ChartNote note = {};

				elementAttr = element->FindAttribute("Time");
				elementAttr->QueryFloatValue(&note.AppearTime);

				elementAttr = element->FindAttribute("Shape");
				const char* shapeText = elementAttr->Value();
				note.Shape = g_NoteShapeConversionTable.at(shapeText);

				elementAttr = element->FindAttribute("Type");
				if (elementAttr != nullptr)
				{
					const char* typeText = elementAttr->Value();
					note.Type = g_NoteTypeConversionTable.at(typeText);
				}

				elementAttr = element->FindAttribute("ReferenceIndex");
				elementAttr->QueryIntValue(&note.ReferenceIndex);

				elementAttr = element->FindAttribute("X");
				elementAttr->QueryFloatValue(&note.Position.x);

				elementAttr = element->FindAttribute("Y");
				elementAttr->QueryFloatValue(&note.Position.y);

				elementAttr = element->FindAttribute("Angle");
				elementAttr->QueryFloatValue(&note.Angle);

				elementAttr = element->FindAttribute("Frequency");
				elementAttr->QueryFloatValue(&note.Frequency);

				elementAttr = element->FindAttribute("Amplitude");
				elementAttr->QueryFloatValue(&note.Amplitude);

				elementAttr = element->FindAttribute("Distance");
				elementAttr->QueryFloatValue(&note.Distance);

				Notes.push_back(note);
			}
			else if (SDL_strncmp(element->Name(), "Event", 6) == 0)
			{
				float execTime = 0.0f;
				std::unordered_map<std::string, ChartEventType>::const_iterator eventType = g_ChartEventTypeConversionTable.begin();

				elementAttr = element->FindAttribute("Time");
				elementAttr->QueryFloatValue(&execTime);

				elementAttr = element->FindAttribute("Type");
				const char* typeText = elementAttr->Value();
				eventType = g_ChartEventTypeConversionTable.find(typeText);

				if (eventType == g_ChartEventTypeConversionTable.end())
				{
					continue;
				}

				switch (eventType->second)
				{
					case ChartEventType::EVENT_SET_BPM:
						{
							SetBPMEvent* setBPMevent = new SetBPMEvent();

							setBPMevent->ExecutionTime = execTime;
							setBPMevent->Type = ChartEventType::EVENT_SET_BPM;

							elementAttr = element->FindAttribute("BPM");
							elementAttr->QueryFloatValue(&setBPMevent->BPM);

							elementAttr = element->FindAttribute("BeatsPerBar");
							elementAttr->QueryIntValue(&setBPMevent->BeatsPerBar);

							Events.push_back(setBPMevent);
							break;
						}
					default:
						{
							ChartEvent* event = new ChartEvent();

							event->ExecutionTime = execTime;
							event->Type = eventType->second;

							if (event->Type == ChartEventType::EVENT_PLAY_MUSIC)
							{
								hasMusicStartCommand = true;
							}

							Events.push_back(event);
						}
						break;
				}
			}
		}

		chartDoc.Clear();
		return true;
	}
	
	bool Chart::LoadFromXml(const std::filesystem::path& path)
	{
		using namespace IO;

		FileSystem* fs = FileSystem::GetInstance();
		fsPath fntPath = fs->GetContentFilePath(path);

		fstream xmlFile;
		xmlFile.open(fntPath, ios::in | ios::binary);

		if (xmlFile.bad())
		{
			return false;
		}

		// Get file size
		size_t xmlSize = std::filesystem::file_size(fntPath);
		char *xmlData = new char[xmlSize + 1];
		xmlData[xmlSize] = '\0';

		xmlFile.read(xmlData, xmlSize);
		xmlFile.close();

		if (!ReadFromXml(xmlData, xmlSize + 1))
		{
			delete[] xmlData;
			return false;
		}

		delete[] xmlData;
		return true;
	}
	
	std::string Chart::WriteToXml()
	{
		if (Notes.empty() || Events.empty())
		{
			return 0;
		}

		XMLPrinter printer;
		printer.OpenElement("Chart");
		
		for (vector<ChartNote>::iterator it = Notes.begin(); it != Notes.end(); it++)
		{
			printer.OpenElement("Note");

			printer.PushAttribute("Time", it->AppearTime);
			printer.PushAttribute("Shape", g_NoteShapeNames[static_cast<int>(it->Shape)]);
			printer.PushAttribute("Type", g_NoteTypeNames[static_cast<int>(it->Type)]);
			printer.PushAttribute("ReferenceIndex", it->ReferenceIndex);

			printer.PushAttribute("X", it->Position.x);
			printer.PushAttribute("Y", it->Position.y);

			printer.PushAttribute("Angle", it->Angle);
			printer.PushAttribute("Frequency", it->Frequency);
			printer.PushAttribute("Amplitude", it->Amplitude);
			printer.PushAttribute("Distance", it->Distance);

			printer.CloseElement();
		}

		for (vector<ChartEvent*>::iterator it = Events.begin(); it != Events.end(); it++)
		{
			printer.OpenElement("Event");

			printer.PushAttribute("Time", (*it)->ExecutionTime);
			printer.PushAttribute("Type", g_EventTypeNames[static_cast<int>((*it)->Type)]);

			switch ((*it)->Type)
			{
			case ChartEventType::EVENT_SET_BPM:
				{
					SetBPMEvent* actualEvent = static_cast<SetBPMEvent*>(*it);	
			
					printer.PushAttribute("BPM", actualEvent->BPM);
					printer.PushAttribute("BeatsPerBar", actualEvent->BeatsPerBar);

					break;
				}
			case ChartEventType::EVENT_SONG_END:
				break;
			}

			printer.CloseElement();
		}

		printer.CloseElement();

		string outputString = string(printer.CStr());
		printer.ClearBuffer();

		return outputString;
	}

	bool Chart::HasMusicStartCommand()
	{
		return hasMusicStartCommand;
	}
}
