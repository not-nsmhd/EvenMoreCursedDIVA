#pragma once
#include "Common/Types.h"

namespace Starshine::GFX
{
	class GPUResource
	{
	public:
		virtual ~GPUResource() = default;

		virtual void SetDebugName(std::string_view name) {};
	};
}
