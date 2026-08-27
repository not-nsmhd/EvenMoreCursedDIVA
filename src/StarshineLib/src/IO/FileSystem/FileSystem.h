#pragma once
#include "Common/Types.h"
#include "IO/FileStream.h"
#include <memory>
#include <vector>

namespace Starshine::IO
{
	class FileSystem : NonCopyable
	{
	public:
		static void CreateInstance();
		static FileSystem* GetInstance();

	public:
		bool AddSearchPath(std::string_view path);
		bool SetWritePath(std::string_view dirPath);

	public:
		bool FileExists(std::string_view path);
		bool DirectoryExists(std::string_view path);

		std::string GetFullFilePath(std::string_view path);
		std::string GetFullDirectoryPath(std::string_view path);

		bool OpenFileStream(FileStream& stream, std::string_view path, bool read, bool write);
		bool CreateFileStream(FileStream& stream, std::string_view path);
		size_t GetFileSize(std::string_view path);

		size_t ReadAllBytes(std::string_view filePath, std::unique_ptr<u8[]>& data);
		size_t ReadAllText(std::string_view filePath, std::unique_ptr<char[]>& data);

	public:
		FileSystem() = default;
		~FileSystem() = default;

	private:
		std::vector<std::string> mountedPaths;
		std::string writePath;
	};
}
