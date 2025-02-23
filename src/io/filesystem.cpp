#include "filesystem.h"
#include <string>
#ifdef _WIN32
#include <shlobj.h>
#include <shlwapi.h>
#endif
#include "../util/logging.h"

using std::string;

namespace IO
{
	FileSystem::FileSystem()
	{
		contentPaths = std::vector<std::filesystem::path>();

#ifdef _WIN32
		CHAR myDocuments[MAX_PATH];
		BOOL result = SHGetSpecialFolderPathA(NULL, myDocuments, CSIDL_MYDOCUMENTS, FALSE);

		if (result == TRUE)
		{
			string docsPath = string(myDocuments);
			string savePath = docsPath;
			savePath.append("\\Starshine\\EvenMoreCursedDIVA");

			Logging::LogInfo("IO::FileSystem", "Save data path: %s", savePath.c_str());

			if (PathFileExistsA(savePath.c_str()) != TRUE)
			{
				Logging::LogWarn("IO::FileSystem", "Save data directory does not exist. Creating...");

				string tempPath = docsPath;

				tempPath.append("\\Starshine");
				if (CreateDirectoryA(tempPath.c_str(), NULL) != TRUE)
				{
					if (GetLastError() != ERROR_ALREADY_EXISTS)
					{
						// uiuiuiuiuiuiu
						// something
					}
				}

				tempPath.append("\\EvenMoreCursedDIVA");
				CreateDirectoryA(tempPath.c_str(), NULL);
			}

			saveDataPath = std::filesystem::path(savePath);
		}
#endif
	}
	
	FileSystem* FileSystem::instance = nullptr;

	FileSystem* FileSystem::GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new FileSystem();
		}
		
		return instance;
	}
	
	bool FileSystem::MountPath(const std::filesystem::path& path)
	{
		std::filesystem::path absPath = std::filesystem::absolute(path);

		if (std::filesystem::exists(absPath))
		{
			contentPaths.push_back(absPath);
			Logging::LogInfo("IO::FileSystem", "Mounted content path \"%s\"", absPath.string().c_str());
			return true;
		}

		return false;
	}
	
	std::filesystem::path FileSystem::GetContentFilePath(const std::filesystem::path& path)
	{
		std::filesystem::path tempPath;
		for (std::vector<std::filesystem::path>::iterator it = contentPaths.begin(); it != contentPaths.end(); it++)
		{
			tempPath = *it;
			tempPath /= (path);

			if (std::filesystem::exists(tempPath))
			{
				return tempPath;
			}
		}

		return path;
	}
	
	std::filesystem::path FileSystem::GetSaveDataFilePath(const std::filesystem::path& path, bool existing)
	{
		std::filesystem::path absPath = saveDataPath;
		absPath /= path;

		if (existing)
		{
			if (std::filesystem::exists(absPath))
			{
				return absPath;
			}

			return std::filesystem::path();
		}

		return absPath;
	}
}