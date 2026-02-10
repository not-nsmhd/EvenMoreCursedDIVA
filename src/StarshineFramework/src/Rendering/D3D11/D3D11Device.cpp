#include "D3D11Device.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>
#include <SDL2/SDL_syswm.h>
#include "Common/Logging/Logging.h"

using namespace Microsoft::WRL;

namespace Starshine::Rendering::D3D11
{
	static constexpr const char* LogName{ "D3D11Device" };

	struct D3D11Device::Impl
	{
		SDL_Window* SDLWindow{};

		struct D3D11Data
		{
			ComPtr<IDXGIFactory2> DXGIFactory{};
			ComPtr<ID3D11Device> Device{};
			ComPtr<ID3D11DeviceContext> DeviceContext{};
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

			if ((result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
				&targetFeatureLevel, 1, D3D11_SDK_VERSION, &D3D11.Device, nullptr, &D3D11.DeviceContext)) != S_OK)
			{
				LogError(LogName, "D3D11CreateDevice failed. Error: 0x%08X", result);
				return false;
			}

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

			return true;
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
		}
	};

	D3D11Device::D3D11Device() : impl(std::make_unique<Impl>())
	{
	}

	D3D11Device::~D3D11Device()
	{
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
	}

	RectangleF D3D11Device::GetViewportSize() const
	{
		return RectangleF();
	}

	void D3D11Device::Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil)
	{
		impl->Clear(flags, color, depth, stencil);
	}

	void D3D11Device::SwapBuffers()
	{
		impl->SwapBuffers();
	}

	void D3D11Device::SetBlendState(bool enable, BlendFactor srcColor, BlendFactor destColor, BlendFactor srcAlpha, BlendFactor destAlpha)
	{
	}

	void D3D11Device::SetBlendOperation(BlendOperation op)
	{
	}

	void D3D11Device::SetFaceCullingState(bool enable, PolygonOrientation backFaceOrientation)
	{
	}

	void D3D11Device::DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
	{
	}

	void D3D11Device::DrawIndexed(PrimitiveType type, u32 firstIndex, u32 vertexCount, u32 indexCount)
	{
	}

	std::unique_ptr<VertexBuffer> D3D11Device::CreateVertexBuffer(size_t size, const void* initialData, bool dynamic)
	{
		return std::unique_ptr<VertexBuffer>();
	}

	std::unique_ptr<IndexBuffer> D3D11Device::CreateIndexBuffer(size_t size, IndexFormat format, const void* initialData, bool dynamic)
	{
		return std::unique_ptr<IndexBuffer>();
	}

	std::unique_ptr<VertexDesc> D3D11Device::CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount)
	{
		return std::unique_ptr<VertexDesc>();
	}

	std::unique_ptr<Shader> D3D11Device::LoadShader(const void* vsData, size_t vsSize, const void* fsData, size_t fsSize)
	{
		return std::unique_ptr<Shader>();
	}

	std::unique_ptr<Texture> D3D11Device::CreateTexture(u32 width, u32 height, GFX::TextureFormat format, bool nearestFilter, bool repeat)
	{
		return std::unique_ptr<Texture>();
	}

	void D3D11Device::SetVertexBuffer(const VertexBuffer* buffer, const VertexDesc* desc)
	{
	}

	void D3D11Device::SetIndexBuffer(const IndexBuffer* buffer)
	{
	}

	void D3D11Device::SetShader(const Shader* shader)
	{
	}

	void D3D11Device::SetTexture(const Texture* texture, u32 slot)
	{
	}
}
