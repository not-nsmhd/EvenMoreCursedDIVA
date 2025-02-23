#pragma once
#include <filesystem>
#include "../lowlevel/shader.h"
#include "../lowlevel/backend.h"

namespace GFX
{
	namespace Helpers
	{
		LowLevel::Shader* LoadShaderFromDescriptor(LowLevel::Backend* backend, const std::filesystem::path& path);
	}
}