#pragma once
#include "Common/Types.h"
#include <vector>

namespace Starshine
{
	class FileDialog
	{
	public:
		std::string_view Title;
		std::string OutputFilePath;

	public:
		bool OpenSave();

	private:
		bool InternalOpenDialog(bool save);
	};
}
