#pragma once
#include "Rendering/Buffers.h"
#include "D3D11Device.h"
#include <d3d11.h>
#include <wrl.h>

namespace Starshine::Rendering::D3D11
{
	struct D3D11VertexBuffer : public VertexBuffer
	{
	public:
		D3D11VertexBuffer(ID3D11Device* device, size_t size, bool dynamic, const void* initialData);
		~D3D11VertexBuffer() override;

	public:
		void SetData(const void* source, size_t offset, size_t size);
		void SetDebugName(std::string_view name);

	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> BaseBuffer{};
		ID3D11DeviceContext* DeviceContext{};

		bool Dynamic{ false };
		size_t Size{ 0 };
	};

	struct D3D11IndexBuffer : public IndexBuffer
	{
	public:
		D3D11IndexBuffer(ID3D11Device* device, IndexFormat format, size_t size, bool dynamic, const void* initialData);
		~D3D11IndexBuffer() override;

	public:
		void SetData(const void* source, size_t offset, size_t size);
		void SetDebugName(std::string_view name);

	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> BaseBuffer{};
		ID3D11DeviceContext* DeviceContext{};

		bool Dynamic{ false };
		size_t Size{ 0 };
		IndexFormat Format{};
	};

	struct D3D11UniformBuffer : public UniformBuffer
	{
	public:
		D3D11UniformBuffer(ID3D11Device* device, size_t size, bool dynamic, const void* initialData);
		~D3D11UniformBuffer() override;

	public:
		void SetData(const void* source, size_t offset, size_t size);
		void SetDebugName(std::string_view name);

	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> BaseBuffer{};
		ID3D11DeviceContext* DeviceContext{};

		bool Dynamic{ false };
		size_t Size{ 0 };
	};
}
