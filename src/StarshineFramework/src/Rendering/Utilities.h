#pragma once
#include "Common/Types.h"
#include "Device.h"

namespace Starshine::Rendering
{
	namespace Utilities
	{
		bool LoadShader(std::string_view vsPath, std::string_view fsPath, std::unique_ptr<Shader>& shader);
	}
}
