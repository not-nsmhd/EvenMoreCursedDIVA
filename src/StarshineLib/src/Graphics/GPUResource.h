#pragma once
#include "Common/Types.h"
#include <memory>

namespace Starshine::Graphics
{
	class GPUResource
	{
	public:
		virtual ~GPUResource() = default;

		virtual void SetDebugName(std::string_view name) {};
	};

	struct ManagedGPUResource
	{
		std::unique_ptr<GPUResource> Resource;
		bool Dynamic{};
		bool Reupload{};
	};
}
