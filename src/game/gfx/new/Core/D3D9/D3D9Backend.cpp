#include <d3d9.h>
#include <SDL2/SDL_syswm.h>
#include "D3D9Backend.h"
#include "util/logging.h"

namespace Starshine::GFX::Core::D3D9
{
	using namespace Logging;
	constexpr const char* LogName = "Starshine::GFX::Core::D3D9";

	struct D3D9Backend::Impl
	{
		SDL_Window* GameWindow = nullptr;
		
		struct D3DBaseState
		{
			IDirect3D9* D3D9 = nullptr;
			IDirect3DDevice9* Device = nullptr;
		} BaseState;

		bool Initialize(SDL_Window* gameWindow)
		{
			HRESULT result = D3D_OK;

			BaseState.D3D9 = Direct3DCreate9(D3D_SDK_VERSION);
			if (BaseState.D3D9 == nullptr)
			{
				LogError(LogName, "Failed to create a Direct3D 9 object");
				return false;
			}

			GameWindow = gameWindow;

			D3DADAPTER_IDENTIFIER9 adapterInfo = {};
			BaseState.D3D9->GetAdapterIdentifier(0, 0, &adapterInfo);

			LogInfo(LogName, "Adapter: %s", adapterInfo.Description);

			SDL_SysWMinfo sysWindowInfo = {};
			SDL_VERSION(&sysWindowInfo.version);

			SDL_GetWindowWMInfo(GameWindow, &sysWindowInfo);
			HWND windowHandle = sysWindowInfo.info.win.window;

			int windowWidth = 0;
			int windowHeight = 0;
			SDL_GetWindowSizeInPixels(GameWindow, &windowWidth, &windowHeight);
			u32 windowFlags = SDL_GetWindowFlags(GameWindow);

			DWORD creationFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			D3DPRESENT_PARAMETERS presentParams = {};
			presentParams.BackBufferWidth = windowWidth;
			presentParams.BackBufferHeight = windowHeight;
			presentParams.BackBufferFormat = D3DFMT_A8R8G8B8;
			presentParams.BackBufferCount = 1;
			presentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
			presentParams.MultiSampleQuality = 0;
			presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentParams.hDeviceWindow = windowHandle;
			presentParams.Windowed = static_cast<BOOL>((windowFlags & SDL_WINDOW_FULLSCREEN) == 0);
			//presentParams.FullScreen_RefreshRateInHz = 0;
			//presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

			if ((result = BaseState.D3D9->CreateDevice(
				D3DADAPTER_DEFAULT, 
				D3DDEVTYPE_HAL, 
				windowHandle, 
				creationFlags, 
				&presentParams, 
				&BaseState.Device)) != D3D_OK)
			{
				char message[512] = {};
				SDL_snprintf(message, 511, "Failed to create a Direct3D 9 device.\nError: %08x", result);

				LogError(LogName, message);
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (GFX)", message, GameWindow);

				return false;
			}

			return true;
		}

		void Destroy()
		{
			BaseState.Device->Release();
			BaseState.D3D9->Release();
		}

		void Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil)
		{
			DWORD clearFlags = 0;
			D3DCOLOR clearColor = 0;

			if ((flags & ClearFlags::ClearFlags_Color) != 0)
			{
				clearFlags |= D3DCLEAR_TARGET;
				clearColor = D3DCOLOR_ARGB(color.A, color.R, color.G, color.B);
			}

			if ((flags & ClearFlags::ClearFlags_Depth) != 0)
			{
				clearFlags |= D3DCLEAR_ZBUFFER;
			}

			if ((flags & ClearFlags::ClearFlags_Stencil) != 0)
			{
				clearFlags |= D3DCLEAR_STENCIL;
			}

			BaseState.Device->Clear(0, NULL, clearFlags, clearColor, depth, stencil);
		}

		void SwapBuffers()
		{
			BaseState.Device->Present(NULL, NULL, NULL, NULL);
		}
	};

	D3D9Backend::D3D9Backend() : impl(new D3D9Backend::Impl())
	{
	}

	D3D9Backend::~D3D9Backend()
	{
	}

	bool D3D9Backend::Initialize(SDL_Window* gameWindow)
	{
		return impl->Initialize(gameWindow);
	}

	void D3D9Backend::Destroy()
	{
		impl->Destroy();
		delete impl;
	}

	RendererBackendType D3D9Backend::GetType() const
	{
		return RendererBackendType::D3D9;
	}

	void D3D9Backend::Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil)
	{
		impl->Clear(flags, color, depth, stencil);
	}

	void D3D9Backend::SwapBuffers()
	{
		impl->SwapBuffers();
	}
}
