#include "FileSystem.h"
#include "IO/Path/File.h"
#include "IO/Path/Directory.h"
#include "Common/Logging/Logging.h"

namespace Starshine::IO
{
	static constexpr const char* LogName = "FileSystem";

	std::unique_ptr<FileSystem> Instance{};

	void FileSystem::CreateInstance()
	{
		Instance = std::make_unique<FileSystem>();
		Instance->AddSearchPath("./");
	}

	FileSystem* FileSystem::GetInstance()
	{
		if (Instance != nullptr)
			return Instance.get();

		return nullptr;
	}

	bool FileSystem::AddSearchPath(std::string_view path)
	{
		if (Directory::Exists(path))
		{
			mountedPaths.emplace_back(path);
			std::string fullDirPath = Directory::GetFullPath(path);
			LogInfo(LogName, "Added search path: %s", fullDirPath.c_str());
			return true;
		}
		
		return false;
	}

	bool FileSystem::SetWritePath(std::string_view dirPath)
	{
		if (Directory::Exists(dirPath))
		{
			writePath = dirPath;
			return true;
		}

		return false;
	}

	bool FileSystem::FileExists(std::string_view path)
	{
		for (auto& mntPath : mountedPaths)
		{
			std::string fullPath = Path::Append(mntPath, path);
			if (File::Exists(fullPath))
				return true;
		}

		return false;
	}

	bool FileSystem::DirectoryExists(std::string_view path)
	{
		for (auto& mntPath : mountedPaths)
		{
			std::string fullPath = Path::Append(mntPath, path);
			if (Directory::Exists(fullPath))
				return true;
		}

		return false;
	}

	std::string FileSystem::GetFullFilePath(std::string_view path)
	{
#ifdef _WIN32
		if (path.size() >= 2)
		{
			// We've got a full file path with the drive letter and all
			if (path[1] == ':')
			{
				if (File::Exists(path))
					return std::string(path);
			}
		}
#endif
		for (auto& mntPath : mountedPaths)
		{
			std::string fullPath = Path::Append(mntPath, path);
			if (File::Exists(fullPath))
				return fullPath;
		}

		return "";
	}

	std::string FileSystem::GetFullDirectoryPath(std::string_view path)
	{
#ifdef _WIN32
		if (path.size() >= 2)
		{
			// We've got a full file path with the drive letter and all
			if (path[1] == ':')
			{
				if (Directory::Exists(path))
					return std::string(path);
			}
		}
#endif

		for (auto& mntPath : mountedPaths)
		{
			std::string fullPath = Path::Append(mntPath, path);
			if (Directory::Exists(fullPath))
				return fullPath;
		}

		return "";
	}

	bool FileSystem::OpenFileStream(FileStream& stream, std::string_view path, bool read, bool write)
	{
		if (write)
		{
			std::string fullPath = Path::Append(writePath, path);
			if (File::Exists(fullPath))
			{
				stream.OpenReadWrite(fullPath);
				return true;
			}
		}
		else if (read && !write)
		{
			std::string fullPath = GetFullFilePath(path);
			if (!fullPath.empty())
			{
				stream.OpenRead(fullPath);
				return true;
			}
		}

		return false;
	}

	bool FileSystem::CreateFileStream(FileStream& stream, std::string_view path)
	{
		stream.CreateWrite(path);
		return true;
	}

	size_t FileSystem::GetFileSize(std::string_view path)
	{
		std::string fullPath = GetFullFilePath(path);
		if (!fullPath.empty())
			return File::GetSize(fullPath);

		return 0;
	}

	size_t FileSystem::ReadAllBytes(std::string_view filePath, std::unique_ptr<u8[]>& data)
	{
		std::string fullPath = GetFullFilePath(filePath);
		if (!fullPath.empty())
			return File::ReadAllBytes(fullPath, data);

		return 0;
	}

	size_t FileSystem::ReadAllText(std::string_view filePath, std::unique_ptr<char[]>& data)
	{
		std::string fullPath = GetFullFilePath(filePath);
		if (!fullPath.empty())
			return File::ReadAllText(fullPath, data);

		return 0;
	}
}
