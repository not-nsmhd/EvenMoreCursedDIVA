#pragma once
#include <filesystem>
#include <vector>

namespace IO
{
	class FileSystem
	{
	protected:
		FileSystem();
		static FileSystem* instance;

	public:
		FileSystem(FileSystem& other) = delete;
		void operator=(const FileSystem&) = delete;

		static FileSystem* GetInstance();

		bool MountPath(const std::filesystem::path& path);

		std::filesystem::path GetContentFilePath(const std::filesystem::path& path);
		std::filesystem::path GetSaveDataFilePath(const std::filesystem::path& path, bool existing);
	private:
		std::vector<std::filesystem::path> contentPaths;
		std::filesystem::path saveDataPath;
	};
}
