#pragma once
#include "Common/Types.h"

namespace Starshine::IO
{
	namespace Path
	{
		constexpr char ExtensionSeparator = '.';

		constexpr char DirectorySeparator = '/';
#ifdef _WIN32
		constexpr char DirectorySeparatorAlt = '\\';
		constexpr const char* DirectorySeparators = "/\\";
#endif // _WIN32

		// NOTE: Extension is returned with a '.' at the beginning
		constexpr std::string_view GetExtension(std::string_view filePath)
		{
			const size_t lastDirIndex = filePath.find_last_of(DirectorySeparators);
			const std::string_view trimmedPath = (lastDirIndex == std::string_view::npos) ? filePath : filePath.substr(lastDirIndex);

			const size_t extensionIndex = trimmedPath.find_last_of(ExtensionSeparator);
			return (extensionIndex == std::string_view::npos) ? "" : trimmedPath.substr(extensionIndex);
		}

		constexpr std::string_view TrimExtension(std::string_view filePath)
		{
			const std::string_view extension = GetExtension(filePath);
			return filePath.substr(0, filePath.size() - extension.size());
		}

		constexpr std::string_view GetFileName(std::string_view filePath, bool includeExtension = true)
		{
			const size_t lastDirIndex = filePath.find_last_of(DirectorySeparators);
			const std::string_view fileName = (lastDirIndex == std::string_view::npos) ? filePath : filePath.substr(lastDirIndex + 1);

			return includeExtension ? fileName : TrimExtension(fileName);
		}

		// NOTE: "newExtension" must contain a '.' at the beginning
		std::string ChangeExtension(std::string_view filePath, std::string_view newExtension);
	}
}
