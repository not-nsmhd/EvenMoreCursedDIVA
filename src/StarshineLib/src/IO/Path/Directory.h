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
			template<bool IterateFiles, bool IterateDirectories, typename Func>
			void Iterate(std::string_view directoryPath, Func func)
			{
				if (Path::GetExtension(directoryPath).empty() != true) { return; }

				auto iterate = [&](const std::filesystem::directory_iterator dirIterator)
				{
					for (const auto& it : dirIterator)
					{
						bool validPath = false;

						if constexpr (IterateFiles) { validPath |= it.is_regular_file(); }
						if constexpr (IterateDirectories) { validPath |= it.is_directory(); }

						if (validPath) { func(it.path().u8string()); }
					}
				};

				iterate(std::filesystem::directory_iterator(directoryPath));
			}
		}

		bool Exists(std::string_view directoryPath);

		template<typename Func>
		void IterateFiles(std::string_view directoryPath, Func func) { Detail::Iterate<true, false>(directoryPath, func); }
	}
}
