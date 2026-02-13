#include "SongInfo.h"
#include <IO/Xml.h>
#include <IO/Path/File.h>

namespace DIVA::Formats
{
	using namespace Starshine;
	using namespace Starshine::IO;

	namespace PropertyNames
	{
		static constexpr std::string_view SongInfo = "SongInfo";
		static constexpr std::string_view SongName = "Name";

		static constexpr std::string_view ChartFileList = "Charts";
		static constexpr std::string_view LyricsFile = "LyricsFile";
		static constexpr std::string_view MusicFile = "MusicFile";
	}

	bool SongInfo::ParseFromFile(std::string_view filePath)
	{
		if (!File::Exists(filePath)) { return false; }

		Xml::Document infoDocument;
		if (!Xml::ParseFromFile(infoDocument, filePath)) { return false; }

		// --------------

		const Xml::Element* rootElement = infoDocument.FirstChildElement("SongInfo");
		if (rootElement == nullptr) { return false; }

		const Xml::Attribute* nameAttribute = rootElement->FindAttribute("Name");
		Name = std::string(nameAttribute->Value());

		// --------------

		const Xml::Element* childElement = rootElement->FirstChildElement();

		auto getAttributeValue = [&](const Xml::Element* element, const char* attribName, std::string& output)
		{
			const Xml::Attribute* attribute = element->FindAttribute(attribName);
			if (attribute != nullptr) { output = std::string(attribute->Value()); }
		};

		while (childElement != nullptr)
		{
			if (!::strncmp(childElement->Name(), PropertyNames::ChartFileList.data(), PropertyNames::ChartFileList.size()))
			{
				for (size_t i = 0; i < EnumCount<ChartDifficulty>(); i++)
				{
					const Xml::Element* chartPathElement = childElement->FirstChildElement(ChartDifficultyNames[i]);
					if (chartPathElement == nullptr) { continue; }

					getAttributeValue(chartPathElement, "Path", ChartFilePaths[i]);
				}
			}
			else if (!::strncmp(childElement->Name(), PropertyNames::LyricsFile.data(), PropertyNames::LyricsFile.size()))
			{
				getAttributeValue(childElement, "Path", LyricsFilePath);
			}
			else if (!::strncmp(childElement->Name(), PropertyNames::MusicFile.data(), PropertyNames::MusicFile.size()))
			{
				getAttributeValue(childElement, "Path", MusicFilePath);
			}

			childElement = childElement->NextSiblingElement();
		}

		infoDocument.Clear();
		return true;
	}
}
