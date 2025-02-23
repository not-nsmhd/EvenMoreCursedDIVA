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
	Chart::Chart()
	{
	}
	
	Chart::~Chart()
	{
		Notes.clear();
	}
	
	void Chart::Clear()
	{
		Notes.clear();
	}
	
	bool Chart::ReadFromXml(const char* xml, size_t size)
	{
		XMLDocument chartDoc;
		chartDoc.Parse(xml, size + 1);

		XMLElement* rootNode = chartDoc.FirstChildElement("Chart");

		const XMLAttribute* elementAttr;
		for (XMLElement* element = rootNode->FirstChildElement(); element; element = element->NextSiblingElement())
		{
			if (strncmp(element->Name(), "Note", 5) == 0)
			{
				ChartNote note = {};

				elementAttr = element->FindAttribute("Time");
				elementAttr->QueryFloatValue(&note.AppearTime);

				elementAttr = element->FindAttribute("Shape");
				const char* shapeText = elementAttr->Value();
				note.Shape = g_NoteShapeConversionTable.at(shapeText);

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
		if (Notes.size() == 0)
		{
			return 0;
		}

		XMLPrinter printer;
		printer.OpenElement("Chart");
		
		for (vector<ChartNote>::iterator it = Notes.begin(); it != Notes.end(); it++)
		{
			printer.OpenElement("Note");

			printer.PushAttribute("Time", it->AppearTime);
			printer.PushAttribute("Shape", g_NoteShapeNames[static_cast<int>(it->Shape)].c_str());
			printer.PushAttribute("ReferenceIndex", it->ReferenceIndex);

			printer.PushAttribute("X", it->Position.x);
			printer.PushAttribute("Y", it->Position.y);

			printer.PushAttribute("Angle", it->Angle);
			printer.PushAttribute("Frequency", it->Frequency);
			printer.PushAttribute("Amplitude", it->Amplitude);
			printer.PushAttribute("Distance", it->Distance);

			printer.CloseElement();
		}

		printer.CloseElement();

		string outputString = string(printer.CStr());
		printer.ClearBuffer();

		return outputString;
	}
}
