#include "D3D11Device.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>
#include <SDL2/SDL_syswm.h>
#include "Common/Logging/Logging.h"
#include "D3D11Shader.h"
#include "D3D11VertexDesc.h"
#include "D3D11Buffers.h"
#include "D3D11Texture.h"
#include "D3D11State.h"

using namespace Microsoft::WRL;

namespace Starshine::Rendering::D3D11
{
	static constexpr const char* LogName{ "D3D11Device" };

	static constexpr std::array<D3D_PRIMITIVE_TOPOLOGY, EnumCount<PrimitiveType>()> D3DPrimitiveTypes
	{
		D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,

		D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,

		D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	};

	static constexpr std::array<DXGI_FORMAT, EnumCount<IndexFormat>()> DXGIIndexFormats
	{
		DXGI_FORMAT::DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT::DXGI_FORMAT_R32_UINT
	};

	struct D3D11Device::Impl
	{
		SDL_Window* SDLWindow{};

		struct D3D11Data
		{
			ComPtr<IDXGIFactory2> DXGIFactory{};
			ComPtr<ID3D11Device> Device{};
			ComPtr<ID3D11DeviceContext> DeviceContext{};

			ComPtr<ID3D11DepthStencilState> DisabledDSState{};
			ComPtr<ID3D11RasterizerState> NoCullRSState{};

#if defined (_DEBUG)
			ComPtr<ID3D11Debug> Debug{};
#endif
		} D3D11;

		struct SwapChainData
		{
			DXGI_SWAP_CHAIN_DESC1 Desc{};
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullScreenDesc{};
			ComPtr<IDXGISwapChain1> DXGISwapChain{};
		} SwapChain;

		struct SwapChainResourcesData
		{
			ComPtr<ID3D11Texture2D> ColorTexture{};
			ComPtr<ID3D11RenderTargetView> RTView{};
			//ComPtr<ID3D11DepthStencilView> DSView{};
		} SwapChainResources;

		D3D11_VIEWPORT CurrentViewport{};
		D3D11_RECT CurrentScissorRect{};

		struct DrawCheckFlagsData
		{
			bool VertexBufferSet{ false };
			bool IndexBufferSet{ false };
			bool VertexDescSet{ false };
			bool ShaderSet{ false };

			UINT CurrentVertexStride{ 0 };
		} DrawCheckFlags;

		Impl()
		{
		}

		~Impl() 
		{
		}

		bool Initialize(SDL_Window* window)
		{
			SDLWindow = window;

			HRESULT result = 0;
			if ((result = CreateDXGIFactory1(IID_PPV_ARGS(&D3D11.DXGIFactory))) != S_OK)
			{
				LogError(LogName, "CreateDXGIFactory1 failed. Error: 0x%08X", result);
				return false;
			}

			static constexpr D3D_FEATURE_LEVEL targetFeatureLevel{ D3D_FEATURE_LEVEL_11_0 };
			static UINT deviceFlags = 0;
#if defined (_DEBUG)
			deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			if ((result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags,
				&targetFeatureLevel, 1, D3D11_SDK_VERSION, &D3D11.Device, nullptr, &D3D11.DeviceContext)) != S_OK)
			{
				LogError(LogName, "D3D11CreateDevice failed. Error: 0x%08X", result);
				return false;
			}

#if defined (_DEBUG)
			if ((result = D3D11.Device.As(&D3D11.Debug)) != S_OK)
			{
				LogError(LogName, "Failed to get device's debug layer. Error: 0x%08X", result);
				return false;
			}

			
#endif

			SDL_SysWMinfo wmInfo{};
			SDL_GetWindowWMInfo(window, &wmInfo);

			i32 width, height{};
			SDL_GetWindowSizeInPixels(window, &width, &height);

			SwapChain.Desc.Width = width;
			SwapChain.Desc.Height = height;
			SwapChain.Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			SwapChain.Desc.SampleDesc.Count = 1;
			SwapChain.Desc.SampleDesc.Quality = 0;
			SwapChain.Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			SwapChain.Desc.BufferCount = 2;
			SwapChain.Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			SwapChain.Desc.Scaling = DXGI_SCALING_STRETCH;
			SwapChain.FullScreenDesc.Windowed = TRUE;

			if ((result = D3D11.DXGIFactory->CreateSwapChainForHwnd(D3D11.Device.Get(), wmInfo.info.win.window,
				&SwapChain.Desc, &SwapChain.FullScreenDesc, nullptr, &SwapChain.DXGISwapChain)) != S_OK)
			{
				LogError(LogName, "DXGIFactory->CreateSwapChainForHwnd failed. Error: 0x%08X", result);
				return false;
			}

			if ((result = SwapChain.DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&SwapChainResources.ColorTexture))) != S_OK)
			{
				LogError(LogName, "DXGISwapChain->GetBuffer failed. Error: 0x%08X", result);
				return false;
			}

			if ((result = D3D11.Device->CreateRenderTargetView(SwapChainResources.ColorTexture.Get(), nullptr, &SwapChainResources.RTView)) != S_OK)
			{
				LogError(LogName, "Device->CreateRenderTargetView failed. Error: 0x%08X", result);
				return false;
			}

			CurrentViewport.TopLeftX = 0.0f;
			CurrentViewport.TopLeftY = 0.0f;
			CurrentViewport.Width = static_cast<FLOAT>(width);
			CurrentViewport.Height = static_cast<FLOAT>(height);
			CurrentViewport.MinDepth = 0.0f;
			CurrentViewport.MaxDepth = 1.0f;

			CurrentScissorRect.left = 0;
			CurrentScissorRect.top = 0;
			CurrentScissorRect.right = width;
			CurrentScissorRect.bottom = height;

			D3D11.DeviceContext->RSSetViewports(1, &CurrentViewport);
			D3D11.DeviceContext->RSSetScissorRects(1, &CurrentScissorRect);
			D3D11.DeviceContext->OMSetRenderTargets(1, SwapChainResources.RTView.GetAddressOf(), nullptr);

			D3D11_DEPTH_STENCIL_DESC dsStateDesc{};
			dsStateDesc.DepthEnable = FALSE;
			dsStateDesc.StencilEnable = FALSE;
			dsStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			dsStateDesc.StencilWriteMask = 0x00;
			D3D11.Device->CreateDepthStencilState(&dsStateDesc, &D3D11.DisabledDSState);
			D3D11.DeviceContext->OMSetDepthStencilState(D3D11.DisabledDSState.Get(), 0);

			D3D11_RASTERIZER_DESC rsStateDesc{};
			rsStateDesc.CullMode = D3D11_CULL_NONE;
			rsStateDesc.ScissorEnable = FALSE;
			rsStateDesc.FillMode = D3D11_FILL_SOLID;
			D3D11.Device->CreateRasterizerState(&rsStateDesc, &D3D11.NoCullRSState);
			D3D11.DeviceContext->RSSetState(D3D11.NoCullRSState.Get());

			return true;
		}

		void Destroy()
		{
			SwapChainResources.ColorTexture.Reset();
			SwapChainResources.RTView.Reset();

			D3D11.DisabledDSState.Reset();
			D3D11.DeviceContext.Reset();

#if defined (_DEBUG)
			D3D11.Debug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
			D3D11.Debug.Reset();
#endif

			D3D11.Device.Reset();
		}

		void Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil)
		{
			if ((flags & ClearFlags_Color) != 0)
			{
				FLOAT d3dColor[4]
				{
					static_cast<FLOAT>(color.R) / 255.0f,
					static_cast<FLOAT>(color.G) / 255.0f,
					static_cast<FLOAT>(color.B) / 255.0f,
					static_cast<FLOAT>(color.A) / 255.0f
				};

				D3D11.DeviceContext->ClearRenderTargetView(SwapChainResources.RTView.Get(), d3dColor);
			}

			bool clearDepth = ((flags & ClearFlags_Depth) != 0);
			bool clearStencil = ((flags & ClearFlags_Stencil) != 0);

			if (clearDepth || clearStencil)
			{
				// TODO: Implement
			}
		}

		void SwapBuffers()
		{
			SwapChain.DXGISwapChain->Present(1, 0);
			D3D11.DeviceContext->OMSetRenderTargets(1, SwapChainResources.RTView.GetAddressOf(), nullptr);
		}

		void OnWindowResize(i32 width, i32 height)
		{
			D3D11.DeviceContext->Flush();

			SwapChainResources.ColorTexture.Reset();
			SwapChainResources.RTView.Reset();

			SwapChain.DXGISwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
			SwapChain.DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&SwapChainResources.ColorTexture));
			D3D11.Device->CreateRenderTargetView(SwapChainResources.ColorTexture.Get(), nullptr, &SwapChainResources.RTView);

			CurrentViewport.TopLeftX = 0.0f;
			CurrentViewport.TopLeftY = 0.0f;
			CurrentViewport.Width = static_cast<FLOAT>(width);
			CurrentViewport.Height = static_cast<FLOAT>(height);
			CurrentViewport.MinDepth = 0.0f;
			CurrentViewport.MaxDepth = 1.0f;

			CurrentScissorRect.left = 0;
			CurrentScissorRect.top = 0;
			CurrentScissorRect.right = width;
			CurrentScissorRect.bottom = height;

			D3D11.DeviceContext->RSSetViewports(1, &CurrentViewport);
			D3D11.DeviceContext->RSSetScissorRects(1, &CurrentScissorRect);

			D3D11.DeviceContext->OMSetDepthStencilState(D3D11.DisabledDSState.Get(), 0);
			D3D11.DeviceContext->RSSetState(D3D11.NoCullRSState.Get());

			D3D11.DeviceContext->OMSetRenderTargets(1, SwapChainResources.RTView.GetAddressOf(), nullptr);

			LogInfo(LogName, "Main render target and viewport have been resized. New size: %dx%d", width, height);
		}

		void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
		{
			if (DrawCheckFlags.VertexBufferSet && DrawCheckFlags.VertexDescSet && DrawCheckFlags.ShaderSet)
			{
				D3D_PRIMITIVE_TOPOLOGY d3dPrimType = D3DPrimitiveTypes[static_cast<size_t>(type)];
				D3D11.DeviceContext->IASetPrimitiveTopology(d3dPrimType);
				D3D11.DeviceContext->Draw(vertexCount, firstVertex);
			}
		}

		void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 baseVertexIndex, u32 indexCount)
		{
			if (DrawCheckFlags.VertexBufferSet && DrawCheckFlags.IndexBufferSet && DrawCheckFlags.VertexDescSet && DrawCheckFlags.ShaderSet)
			{
				D3D_PRIMITIVE_TOPOLOGY d3dPrimType = D3DPrimitiveTypes[static_cast<size_t>(type)];
				D3D11.DeviceContext->IASetPrimitiveTopology(d3dPrimType);
				D3D11.DeviceContext->DrawIndexed(indexCount, firstIndex, baseVertexIndex);
			}
		}

		void SetVertexBuffer(const D3D11VertexBuffer* buffer, const D3D11VertexDesc* desc)
		{
			if (buffer == nullptr || desc == nullptr)
			{
				DrawCheckFlags.CurrentVertexStride = 0;
				D3D11.DeviceContext->IASetInputLayout(nullptr);
				D3D11.DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
			}
			else
			{
				DrawCheckFlags.CurrentVertexStride = desc->VertexStride;
				UINT vertexOffset = 0;

				D3D11.DeviceContext->IASetInputLayout(desc->InputLayout.Get());
				D3D11.DeviceContext->IASetVertexBuffers(0, 1, buffer->BaseBuffer.GetAddressOf(), &DrawCheckFlags.CurrentVertexStride, &vertexOffset);
			}

			DrawCheckFlags.VertexBufferSet = (buffer != nullptr);
			DrawCheckFlags.VertexDescSet = (desc != nullptr);
		}

		void SetIndexBuffer(const D3D11IndexBuffer* buffer)
		{
			if (buffer == nullptr)
			{
				D3D11.DeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
			}
			else
			{
				DXGI_FORMAT indexFormat = DXGIIndexFormats[static_cast<size_t>(buffer->Format)];
				D3D11.DeviceContext->IASetIndexBuffer(buffer->BaseBuffer.Get(), indexFormat, 0);
			}

			DrawCheckFlags.IndexBufferSet = (buffer != nullptr);
		}

		void SetUniformBuffer(const D3D11UniformBuffer* buffer, ShaderStage stage, u32 bufferIndex)
		{
			if (buffer == nullptr)
			{
				switch (stage)
				{
				case ShaderStage::Vertex:
					D3D11.DeviceContext->VSSetConstantBuffers(bufferIndex, 1, nullptr);
					break;
				case ShaderStage::Fragment:
					D3D11.DeviceContext->PSSetConstantBuffers(bufferIndex, 1, nullptr);
					break;
				}
			}
			else
			{
				switch (stage)
				{
				case ShaderStage::Vertex:
					D3D11.DeviceContext->VSSetConstantBuffers(bufferIndex, 1, buffer->BaseBuffer.GetAddressOf());
					break;
				case ShaderStage::Fragment:
					D3D11.DeviceContext->PSSetConstantBuffers(bufferIndex, 1, buffer->BaseBuffer.GetAddressOf());
					break;
				}
			}
		}

		void SetShader(const D3D11Shader* shader)
		{
			if (shader == nullptr)
			{
				D3D11.DeviceContext->VSSetShader(nullptr, nullptr, 0);
				D3D11.DeviceContext->PSSetShader(nullptr, nullptr, 0);
			}
			else
			{
				D3D11.DeviceContext->VSSetShader(shader->VertexShader.Get(), nullptr, 0);
				D3D11.DeviceContext->PSSetShader(shader->FragmentShader.Get(), nullptr, 0);
			}

			DrawCheckFlags.ShaderSet = (shader != nullptr);
		}

		void SetTexture(const D3D11Texture* texture, u32 slot)
		{
			if (texture == nullptr)
			{
				D3D11.DeviceContext->PSSetShaderResources(slot, 1, nullptr);
				D3D11.DeviceContext->PSSetSamplers(slot, 1, nullptr);
			}
			else
			{
				D3D11.DeviceContext->PSSetShaderResources(slot, 1, texture->ShaderResourceView.GetAddressOf());
				D3D11.DeviceContext->PSSetSamplers(slot, 1, texture->Sampler.GetAddressOf());
			}
		}

		void SetBlendState(const D3D11BlendState* state)
		{
			static constexpr UINT sampleMask = 0xFFFFFFFF;
			if (state == nullptr)
			{
				D3D11.DeviceContext->OMSetBlendState(nullptr, nullptr, sampleMask);
			}
			else
			{
				D3D11.DeviceContext->OMSetBlendState(state->BlendStateObject.Get(), nullptr, sampleMask);
			}
		}
	};

	D3D11Device::D3D11Device() : impl(std::make_unique<Impl>())
	{
	}

	D3D11Device::~D3D11Device()
	{
	}

	ID3D11Device* D3D11Device::GetBaseDevice()
	{
		return impl->D3D11.Device.Get();
	}

	bool D3D11Device::Initialize(SDL_Window* gameWindow)
	{
		return impl->Initialize(gameWindow);
	}

	void D3D11Device::Destroy()
	{
	}

	void D3D11Device::OnWindowResize(i32 width, i32 height)
	{
		impl->OnWindowResize(width, height);
	}

	RectangleF D3D11Device::GetViewportSize() const
	{
		UINT viewportIndex{ 1 };
		D3D11_VIEWPORT viewport{};
		impl->D3D11.DeviceContext->RSGetViewports(&viewportIndex, &viewport);
		return RectangleF(viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height);
	}

	void D3D11Device::Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil)
	{
		impl->Clear(flags, color, depth, stencil);
	}

	void D3D11Device::SwapBuffers()
	{
		impl->SwapBuffers();
	}

	void D3D11Device::DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
	{
		impl->DrawArrays(type, firstVertex, vertexCount);
	}

	void D3D11Device::DrawIndexed(PrimitiveType type, u32 firstIndex, u32 baseVertexIndex, u32 indexCount)
	{
		impl->DrawIndexed(type, firstIndex, baseVertexIndex, indexCount);
	}

	std::unique_ptr<VertexBuffer> D3D11Device::CreateVertexBuffer(size_t size, const void* initialData, bool dynamic)
	{
		if ((!dynamic && initialData == nullptr) || (size == 0)) { return nullptr; }

		std::unique_ptr<D3D11VertexBuffer> vertexBuffer = std::make_unique<D3D11VertexBuffer>(GetBaseDevice(), size, dynamic, initialData);
		return vertexBuffer;
	}

	std::unique_ptr<IndexBuffer> D3D11Device::CreateIndexBuffer(size_t size, IndexFormat format, const void* initialData, bool dynamic)
	{
		if ((!dynamic && initialData == nullptr) || (size == 0)) { return nullptr; }

		std::unique_ptr<D3D11IndexBuffer> indexBuffer = std::make_unique<D3D11IndexBuffer>(GetBaseDevice(), format, size, dynamic, initialData);
		return indexBuffer;
	}

	std::unique_ptr<UniformBuffer> D3D11Device::CreateUniformBuffer(size_t size, const void* initialData, bool dynamic)
	{
		if (size == 0) { return nullptr; }

		std::unique_ptr<D3D11UniformBuffer> uniformBuffer = std::make_unique<D3D11UniformBuffer>(GetBaseDevice(), size, dynamic, initialData);
		return uniformBuffer;
	}

	std::unique_ptr<Shader> D3D11Device::LoadShader(const void* vsData, size_t vsSize, const void* fsData, size_t fsSize)
	{
		std::unique_ptr<D3D11Shader> shader = std::make_unique<D3D11Shader>(GetBaseDevice(),
			D3D11ShaderConstBytecode{ reinterpret_cast<const u8*>(vsData), vsSize }, D3D11ShaderConstBytecode{ reinterpret_cast<const u8*>(fsData), fsSize });

		if (!shader->IsUsable()) { return nullptr; }
		return shader;
	}

	std::unique_ptr<VertexDesc> D3D11Device::CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount, const Shader* shader)
	{
		if (shader == nullptr || attribs == nullptr || attribCount == 0 || attribCount > 8) { return nullptr; }

		const D3D11Shader* d3dShader = static_cast<const D3D11Shader*>(shader);
		std::unique_ptr<D3D11VertexDesc> vertexDesc = std::make_unique<D3D11VertexDesc>(
			GetBaseDevice(), attribs, attribCount, d3dShader->VertexShaderBytecode);

		return vertexDesc;
	}

	std::unique_ptr<Texture> D3D11Device::CreateTexture(i32 width, i32 height, GFX::TextureFormat format, const void* initialData)
	{
		if (initialData == nullptr) { return nullptr; }
		std::unique_ptr<D3D11Texture> d3dTexture = std::make_unique<D3D11Texture>(GetBaseDevice(), width, height, format, initialData, false);

		return d3dTexture;
	}

	std::unique_ptr<BlendState> D3D11Device::CreateBlendState(const BlendStateDesc& desc)
	{
		return std::make_unique<D3D11BlendState>(GetBaseDevice(), desc);
	}

	void D3D11Device::SetVertexBuffer(const VertexBuffer* buffer, const VertexDesc* desc)
	{
		if (buffer == nullptr || desc == nullptr)
		{
			impl->SetVertexBuffer(nullptr, nullptr);
		}
		else
		{
			const D3D11VertexBuffer* d3dBuffer = static_cast<const D3D11VertexBuffer*>(buffer);
			const D3D11VertexDesc* d3dVtxDesc = static_cast<const D3D11VertexDesc*>(desc);
			impl->SetVertexBuffer(d3dBuffer, d3dVtxDesc);
		}
	}

	void D3D11Device::SetIndexBuffer(const IndexBuffer* buffer)
	{
		if (buffer == nullptr)
		{
			impl->SetIndexBuffer(nullptr);
		}
		else
		{
			const D3D11IndexBuffer* d3dBuffer = static_cast<const D3D11IndexBuffer*>(buffer);
			impl->SetIndexBuffer(d3dBuffer);
		}
	}

	void D3D11Device::SetUniformBuffer(const UniformBuffer* buffer, ShaderStage stage, u32 bufferIndex)
	{
		if (buffer == nullptr)
		{
			impl->SetUniformBuffer(nullptr, stage, bufferIndex);
		}
		else
		{
			const D3D11UniformBuffer* d3dBuffer = static_cast<const D3D11UniformBuffer*>(buffer);
			impl->SetUniformBuffer(d3dBuffer, stage, bufferIndex);
		}
	}

	void D3D11Device::SetShader(const Shader* shader)
	{
		if (shader != nullptr)
		{
			const D3D11Shader* d3dShader = static_cast<const D3D11Shader*>(shader);
			impl->SetShader(d3dShader);
		}
		else
		{
			impl->SetShader(nullptr);
		}
	}

	void D3D11Device::SetTexture(const Texture* texture, u32 slot)
	{
		if (texture == nullptr)
		{
			impl->SetTexture(nullptr, 0);
		}
		else
		{
			const D3D11Texture* d3dTexture = static_cast<const D3D11Texture*>(texture);
			impl->SetTexture(d3dTexture, slot);
		}
	}

	void D3D11Device::SetBlendState(const BlendState* state)
	{
		if (state == nullptr)
		{
			impl->SetBlendState(nullptr);
		}
		else
		{
			const D3D11BlendState* d3dState = static_cast<const D3D11BlendState*>(state);
			impl->SetBlendState(d3dState);
		}
	}
}
