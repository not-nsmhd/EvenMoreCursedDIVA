#pragma once
#include <filesystem>
#include "../lowlevel/texture.h"
#include "../lowlevel/backend.h"

namespace GFX
{
	namespace Helpers
	{
		LowLevel::Texture* LoadTexture(LowLevel::Backend* backend, const std::filesystem::path& path);
	}
}