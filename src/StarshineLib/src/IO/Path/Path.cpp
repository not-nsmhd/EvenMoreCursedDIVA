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
	}
}
