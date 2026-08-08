#include "Lyrics.h"
#include <IO/Path/File.h>
#include <IO/Xml.h>

namespace DIVA::MainGame
{
	using namespace Starshine;
	using namespace Starshine::IO;

	namespace Lyrics
	{
		bool LoadXml(std::string_view filePath, std::vector<Lyric>& lyricsList)
		{
			if (!File::Exists(filePath)) { return false; }
			
			Xml::Document doc;
			if (!Xml::ParseFromFile(doc, filePath)) { return false; }

			Xml::Element* root = doc.FirstChildElement("Lyrics");
			if (root == nullptr)
			{
				doc.Clear();
				return false;
			}

			// --------------

			const Xml::Element* child = root->FirstChildElement("Lyric");

			while (child != nullptr)
			{
				const char* text = child->GetText();
				if (text == nullptr)
				{
					child = child->NextSiblingElement("Lyric");
					continue;
				}

				Lyric newLyric{};

				const Xml::Attribute* attribute = child->FindAttribute("Start");
				attribute->QueryFloatValue(&newLyric.StartTime);

				attribute = child->FindAttribute("End");
				attribute->QueryFloatValue(&newLyric.EndTime);

				attribute = child->FindAttribute("Color");
				newLyric.Color = Xml::TryGetHexColor(attribute);

				newLyric.Text = text;

				lyricsList.push_back(newLyric);
				child = child->NextSiblingElement("Lyric");
			}
			
			// --------------

			doc.Clear();
			return true;
		}
	}
}
