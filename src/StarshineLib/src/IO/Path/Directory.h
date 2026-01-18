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
			void Iterate(std::string_view directoryPath, Func func)
			{
				if (Path::GetExtension(directoryPath).empty() != true) { return; }

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

				if constexpr (IterateRecursive) { iterate(std::filesystem::recursive_directory_iterator(directoryPath)); }
				else { iterate(std::filesystem::directory_iterator(directoryPath)); }
			}
		}

		bool Exists(std::string_view directoryPath);

		template<typename Func>
		void IterateFiles(std::string_view directoryPath, Func func) { Detail::Iterate<true, false, false>(directoryPath, func); }

		template<typename Func>
		void IterateFilesRecursive(std::string_view directoryPath, Func func) { Detail::Iterate<true, false, true>(directoryPath, func); }
	}
}
