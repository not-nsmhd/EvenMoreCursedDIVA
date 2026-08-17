#include "Path.h"

namespace Starshine::IO
{
	namespace Path
	{
		std::string ChangeExtension(std::string_view filePath, std::string_view newExtension)
		{
			const std::string_view oldExtension = GetExtension(filePath);

			std::string newPath = std::string();
			newPath.reserve(filePath.size() - oldExtension.size() + newExtension.size());
			newPath += filePath.substr(0, filePath.size() - oldExtension.size());
			newPath += newExtension;

			return newPath;
		}

		std::string GetNormalizedPath(std::string_view path)
		{
			std::string result(path);

			for (auto& c : result)
			{
				if (c == '\\')
					c = '/';
			}

			return result;
		}

		std::string Append(std::string_view originalPath, std::string_view appendPath)
		{
			if (originalPath.empty())
				return std::string(appendPath);

			std::string result(originalPath);

			result += DirectorySeparator;
			result += appendPath;

			return result;
		}
	}
}
