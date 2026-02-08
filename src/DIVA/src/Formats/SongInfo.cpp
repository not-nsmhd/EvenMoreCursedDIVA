#include "SongInfo.h"
#include <IO/Xml.h>
#include <IO/Path/File.h>

namespace DIVA::Formats
{
	using namespace Starshine;
	using namespace Starshine::IO;

	bool SongInfo::ParseFromFile(std::string_view filePath)
	{
		if (!File::Exists(filePath)) { return false; }

		std::unique_ptr<u8[]> fileData;
		size_t fileSize = File::ReadAllBytes(filePath, fileData);

		if (fileSize == 0 || fileData == nullptr) { return false; }

		Xml::Document infoDocument;
		if (!Xml::Parse(infoDocument, reinterpret_cast<const char*>(fileData.get()), fileSize)) { return false; }

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
			if (!::strncmp(childElement->Name(), "Charts", 8))
			{
				for (size_t i = 0; i < EnumCount<ChartDifficulty>(); i++)
				{
					const Xml::Element* chartPathElement = childElement->FirstChildElement(ChartDifficultyNames[i]);
					if (chartPathElement == nullptr) { continue; }

					getAttributeValue(chartPathElement, "Path", ChartFilePaths[i]);
				}
			}
			else if (!::strncmp(childElement->Name(), "MusicFile", 11))
			{
				getAttributeValue(childElement, "Path", MusicFilePath);
			}

			childElement = childElement->NextSiblingElement();
		}

		infoDocument.Clear();
		return true;
	}
}
