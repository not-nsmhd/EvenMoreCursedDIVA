#pragma once
#include "Rendering/Buffers.h"
#include "D3D11Device.h"
#include <d3d11.h>
#include <wrl.h>

namespace Starshine::Rendering::D3D11
{
	struct D3D11Buffer : public Buffer
	{
	public:
		D3D11Buffer(D3D11Device& device, const BufferCreationData& props);
		~D3D11Buffer() override;

	public:
		void SetData(const void* source, size_t offset, size_t size);
		void SetDebugName(std::string_view name);

	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> BaseBuffer{};
		ID3D11DeviceContext* DeviceContext{};

		BufferCreationData Properties{};

		D3D11Device& deviceRef;
	};
}
