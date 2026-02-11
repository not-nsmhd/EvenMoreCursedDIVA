#include "D3D11Buffers.h"

namespace Starshine::Rendering::D3D11
{
	D3D11VertexBuffer::D3D11VertexBuffer(ID3D11Device* device, size_t size, bool dynamic, const void* initialData)
		: Dynamic(dynamic), Size(size)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = static_cast<UINT>(size);
		bufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA subresData{};
		if (initialData != nullptr) { subresData.pSysMem = initialData; }

		device->CreateBuffer(&bufferDesc, initialData != nullptr ? &subresData : nullptr, &BaseBuffer);
		device->GetImmediateContext(&DeviceContext);
	}

	D3D11VertexBuffer::~D3D11VertexBuffer()
	{
		BaseBuffer.Reset();
	}

	void D3D11VertexBuffer::SetData(const void* source, size_t offset, size_t size)
	{
		if (Dynamic && (offset + size) <= Size)
		{
			D3D11_MAPPED_SUBRESOURCE mappedSubres{};
			HRESULT result = DeviceContext->Map(BaseBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubres);

			if (result == S_OK)
			{
				::memcpy(&(reinterpret_cast<u8*>(mappedSubres.pData))[offset], source, size);
			}

			DeviceContext->Unmap(BaseBuffer.Get(), 0);
		}
	}

	void D3D11VertexBuffer::SetDebugName(std::string_view name)
	{
#if defined (_DEBUG)
		BaseBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.data());
#endif
	}

	D3D11IndexBuffer::D3D11IndexBuffer(ID3D11Device* device, IndexFormat format, size_t size, bool dynamic, const void* initialData)
		: Dynamic(dynamic), Size(size), Format(format)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = static_cast<UINT>(size);
		bufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA subresData{};
		if (initialData != nullptr) { subresData.pSysMem = initialData; }

		device->CreateBuffer(&bufferDesc, initialData != nullptr ? &subresData : nullptr, &BaseBuffer);
		device->GetImmediateContext(&DeviceContext);
	}

	D3D11IndexBuffer::~D3D11IndexBuffer()
	{
		BaseBuffer.Reset();
	}

	void D3D11IndexBuffer::SetData(const void* source, size_t offset, size_t size)
	{
		if (Dynamic && (offset + size) <= Size)
		{
			D3D11_MAPPED_SUBRESOURCE mappedSubres{};
			HRESULT result = DeviceContext->Map(BaseBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubres);

			if (result == S_OK)
			{
				::memcpy(&(reinterpret_cast<u8*>(mappedSubres.pData))[offset], source, size);
			}

			DeviceContext->Unmap(BaseBuffer.Get(), 0);
		}
	}

	void D3D11IndexBuffer::SetDebugName(std::string_view name)
	{
#if defined (_DEBUG)
		BaseBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.data());
#endif
	}

	D3D11UniformBuffer::D3D11UniformBuffer(ID3D11Device* device, size_t size, bool dynamic, const void* initialData)
		: Dynamic(dynamic), Size(size)
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = static_cast<UINT>(size);
		bufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		D3D11_SUBRESOURCE_DATA subresData{};
		if (initialData != nullptr) { subresData.pSysMem = initialData; }

		device->CreateBuffer(&bufferDesc, initialData != nullptr ? &subresData : nullptr, &BaseBuffer);
		device->GetImmediateContext(&DeviceContext);
	}

	D3D11UniformBuffer::~D3D11UniformBuffer()
	{
		BaseBuffer.Reset();
	}

	void D3D11UniformBuffer::SetData(const void* source, size_t offset, size_t size)
	{
		if ((offset + size) <= Size)
		{
			if (Dynamic)
			{
				D3D11_MAPPED_SUBRESOURCE mappedSubres{};
				HRESULT result = DeviceContext->Map(BaseBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubres);

				if (result == S_OK)
				{
					::memcpy(&(reinterpret_cast<u8*>(mappedSubres.pData))[offset], source, size);
				}

				DeviceContext->Unmap(BaseBuffer.Get(), 0);
			}
			else if (offset == 0 && size == Size)
			{
				DeviceContext->UpdateSubresource(BaseBuffer.Get(), 0, NULL, source, 0, 0);
			}
		}
	}

	void D3D11UniformBuffer::SetDebugName(std::string_view name)
	{
#if defined (_DEBUG)
		BaseBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.data());
#endif
	}
}
