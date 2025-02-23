#include "shader_helpers.h"
#include "../../io/filesystem.h"
#include <cstring>
#include <fstream>
#include <tinyxml2.h>

using std::fstream;
using std::ios;
using namespace IO;
using namespace tinyxml2;

namespace GFX
{
	namespace Helpers
	{
		LowLevel::Shader* LoadShaderFromDescriptor(LowLevel::Backend* backend, const std::filesystem::path& path)
		{
			if (path.empty() || backend == nullptr)
			{
				return nullptr;
			}

			FileSystem* fs = FileSystem::GetInstance();
			std::filesystem::path fullPath = fs->GetContentFilePath(path);

			if (fullPath.empty())
			{
				return nullptr;
			}

			const std::filesystem::path& parentDir = fullPath.parent_path();
			size_t xmlFileSize = std::filesystem::file_size(fullPath);

			fstream xmlFile;
			xmlFile.open(fullPath, ios::in | ios::binary);

			if (xmlFile.bad())
			{
				return nullptr;
			}

			char* xmlData = new char[xmlFileSize + 1];
			xmlFile.read(xmlData, xmlFileSize);
			xmlData[xmlFileSize] = '\0';
			xmlFile.close();

			XMLDocument doc;
			doc.Parse(xmlData, xmlFileSize);

			std::filesystem::path vsPath = parentDir;
			std::filesystem::path fsPath = parentDir;

			const char* currentBackendName = LowLevel::BackendNames[static_cast<int>(backend->GetType())].c_str();

			XMLElement* rootNode = doc.FirstChildElement("Shader");
			for (XMLElement* node = rootNode->FirstChildElement(); node; node = node->NextSiblingElement())
			{
				if (strncmp(node->Name(), "Files", 6) == 0)
				{
					const XMLAttribute* backendAttr = node->FindAttribute("Backend");

					if (strncmp(backendAttr->Value(), currentBackendName, 16) == 0)
					{
						for (XMLElement* pathNode = node->FirstChildElement(); pathNode; pathNode = pathNode->NextSiblingElement())
						{
							if (strncmp(pathNode->Name(), "Vertex", 8) == 0)
							{
								vsPath /= pathNode->GetText();
							}
							else if (strncmp(pathNode->Name(), "Fragment", 10) == 0)
							{
								fsPath /= pathNode->GetText();
							}
						}
					}
				}
			}

			doc.Clear();
			delete[] xmlData;

			// -----------------------

			u8* vsData = nullptr;
			size_t vsSize = 0;

			u8* fsData = nullptr;
			size_t fsSize = 0;

			fstream shaderFile;
			shaderFile.open(vsPath, ios::in | ios::binary);

			if (shaderFile.bad())
			{
				return nullptr;
			}

			vsSize = std::filesystem::file_size(vsPath);
			vsData = new u8[vsSize];
			shaderFile.read((char*)vsData, vsSize);
			shaderFile.close();

			shaderFile.open(fsPath, ios::in | ios::binary);

			if (shaderFile.bad())
			{
				delete[] vsData;
				return nullptr;
			}

			fsSize = std::filesystem::file_size(fsPath);
			fsData = new u8[fsSize];
			shaderFile.read((char*)fsData, fsSize);
			shaderFile.close();

			LowLevel::Shader* self = backend->CreateShader(vsData, vsSize, fsData, fsSize);

			// -----------------------

			delete[] vsData;
			delete[] fsData;
			return self;
		}
	}
}
