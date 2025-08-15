#pragma once
#include "Resource.h"

namespace Starshine::GFX
{
	struct VertexBuffer : public Resource
	{
	public:
		VertexBuffer() : Resource(ResourceType::VertexBuffer) {}

		void* Data = nullptr;
		size_t Size = 0;

		bool Dynamic = false;

		virtual void SetData(void* source, size_t offset, size_t size) = 0;
	};

	struct IndexBuffer : public Resource
	{
	public:
		IndexBuffer() : Resource(ResourceType::IndexBuffer) {}

		void* Data = nullptr;
		size_t Size = 0;

		bool Dynamic = false;
		IndexFormat Format = {};

		virtual void SetData(void* source, size_t offset, size_t size) = 0;
	};
}
