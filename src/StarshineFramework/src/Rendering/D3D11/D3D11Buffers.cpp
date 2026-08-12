#include "D3D11Buffers.h"
#include "Common/MathExt.h"

namespace Starshine::Rendering::D3D11
{
	D3D11Buffer::D3D11Buffer(D3D11Device& device, const BufferCreationData& props)
		: deviceRef(device), Properties(props)
	{
		if (!props.Dynamic) assert(props.InitialData != nullptr);

		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.ByteWidth = static_cast<UINT>(props.Size);
		bufferDesc.Usage = props.Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = props.Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		
		switch (props.Type)
		{
		case BufferType::Vertex:
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case BufferType::Index:
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case BufferType::Uniform:
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			// NOTE: Buffer size for constant buffers must be set to a multiple of 16
			// (https://learn.microsoft.com/en-us/windows/win32/api/D3D11/nf-d3d11-id3d11device-createbuffer#remarks)
			bufferDesc.ByteWidth = static_cast<UINT>(MathExtensions::GetAlignedSize(props.Size, 16));
			break;
		}

		D3D11_SUBRESOURCE_DATA subresData{};
		if (props.InitialData != nullptr) { subresData.pSysMem = props.InitialData; }

		ID3D11Device* d3dDev = device.GetBaseDevice();
		HRESULT result = d3dDev->CreateBuffer(&bufferDesc, props.InitialData != nullptr ? &subresData : nullptr, BaseBuffer.GetAddressOf());
		d3dDev->GetImmediateContext(&DeviceContext);
	}

	D3D11Buffer::~D3D11Buffer()
	{
		deviceRef.QueueObjectForDeletion(BaseBuffer);
	}

	void D3D11Buffer::SetData(const void* source, size_t offset, size_t size)
	{
		if (Properties.Dynamic && (offset + size) <= Properties.Size)
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

	void D3D11Buffer::SetDebugName(std::string_view name)
	{
#if defined (_DEBUG)
		BaseBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.data());
#endif
	}
}
