#pragma once
#include "Common/Types.h"
#include "Path.h"
#include <filesystem>

namespace Starshine::IO
{
	namespace Directory
	{
		namespace Detail
		{
			template<bool IterateFiles, bool IterateDirectories, bool IterateRecursive, typename Func>
			void Iterate(std::string_view dirPath, Func func)
			{
				if (Path::GetExtension(dirPath).empty() != true) { return; }

				auto iterate = [&](const auto dirIterator)
				{
					for (const auto& it : dirIterator)
					{
						bool validPath = false;

						if constexpr (IterateFiles) { validPath |= it.is_regular_file(); }
						if constexpr (IterateDirectories) { validPath |= it.is_directory(); }

						if (validPath) { func(it.path().u8string()); }
					}
				};

				if constexpr (IterateRecursive) { iterate(std::filesystem::recursive_directory_iterator(dirPath)); }
				else { iterate(std::filesystem::directory_iterator(dirPath)); }
			}
		}

		bool Exists(std::string_view dirPath);

		template<typename Func>
		void IterateFiles(std::string_view dirPath, Func func) { Detail::Iterate<true, false, false>(dirPath, func); }

		template<typename Func>
		void IterateFilesRecursive(std::string_view dirPath, Func func) { Detail::Iterate<true, false, true>(dirPath, func); }

		std::string GetFullPath(std::string_view dirPath);
	}
}
