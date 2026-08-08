#pragma once
#include "Common/Types.h"
#include "Types.h"
#include "Graphics/GPUResource.h"

namespace Starshine::Rendering
{
	struct BufferCreationData
	{
		BufferType Type{};
		bool Dynamic{};

		IndexFormat IndexFormat{}; // Used only for index buffers
		size_t Size{};
		const void* InitialData{};
	};

	struct Buffer : public Graphics::GPUResource, NonCopyable
	{
	public:
		Buffer() = default;
		~Buffer() = default;

		virtual void SetData(const void* source, size_t offset, size_t size) = 0;
	};
}
