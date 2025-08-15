#if 0
#include <d3d9.h>
#include <SDL2/SDL_syswm.h>
#include "D3D9Backend.h"
#include <vector>
#include "util/logging.h"

namespace Starshine::GFX::Core::D3D9
{
	using namespace Logging;
	using std::vector;

	constexpr const char* LogName = "Starshine::GFX::Core::D3D9";

	struct ResourceContext
	{
		IDirect3DResource9* BaseResource = nullptr;
		size_t Size = 0;
	};

	struct D3D9Backend::Impl
	{
		SDL_Window* GameWindow = nullptr;
		
		struct D3DBaseState
		{
			IDirect3D9* D3D9 = nullptr;
			IDirect3DDevice9* Device = nullptr;
		} BaseState;

		vector<ResourceContext> Resources;

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

			constexpr size_t reasonableInitialResources = 512;
			Resources.reserve(reasonableInitialResources);

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

		ResourceHandle FindFreeResourceContext()
		{
			for (size_t i = 0; i < Resources.size(); i++)
			{
				ResourceContext& ctx = Resources[i];

				if (ctx.BaseResource == nullptr && ctx.Size == 0)
				{
					return i;
				}
			}

			Resources.emplace_back();
			return static_cast<ResourceHandle>(Resources.size() - 1);
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

	VertexBuffer* D3D9Backend::CreateVertexBuffer(size_t size, void* initialData, bool dynamic)
	{
		if (!dynamic && initialData == nullptr || size == 0)
		{
			return nullptr;
		}

		IDirect3DDevice9* d3dDevice = impl->BaseState.Device;
		IDirect3DVertexBuffer9* d3dBuffer = nullptr;
		HRESULT result = D3D_OK;

		UINT bufferLength = static_cast<UINT>(size);
		DWORD bufferUsage = (dynamic) ? D3DUSAGE_DYNAMIC : D3DUSAGE_WRITEONLY;
		if ((result = d3dDevice->CreateVertexBuffer(bufferLength, bufferUsage, 0, D3DPOOL_DEFAULT, &d3dBuffer, NULL)) != D3D_OK)
		{
			LogError(LogName, "Failed to create a new vertex buffer. Error: %08x", result);
			return nullptr;
		}

		void* bufferData = new u8[size];

		if (initialData != nullptr)
		{
			SDL_memcpy(bufferData, initialData, size);

			void* d3dBufferData = nullptr;
			d3dBuffer->Lock(0, bufferLength, &d3dBufferData, D3DLOCK_DISCARD);
			SDL_memcpy(d3dBufferData, bufferData, size);
			d3dBuffer->Unlock();
		}
		else
		{
			SDL_memset(bufferData, 0, size);
		}

		ResourceHandle handle = impl->FindFreeResourceContext();
		ResourceContext* ctx = &impl->Resources.at(static_cast<size_t>(handle));

		ctx->BaseResource = d3dBuffer;
		ctx->Size = size;

		VertexBuffer_D3D9* buffer = new VertexBuffer_D3D9();
		buffer->Handle = handle;
		buffer->Type = ResourceType::VertexBuffer;
		buffer->Context = ctx;

		buffer->Data = bufferData;
		buffer->Size = size;
		buffer->Dynamic = dynamic;

		LogInfo(LogName, "Created a new vertex buffer (handle %d)", handle);
		return buffer;
	}

	void D3D9Backend::DeleteResource(Resource* resource)
	{
		if (resource->Handle == InvalidResourceHandle)
		{
			return;
		}

		ResourceContext* resourceCtx = &impl->Resources.at(static_cast<size_t>(resource->Handle));
		resourceCtx->BaseResource->Release();
		resourceCtx->BaseResource = nullptr;
		resourceCtx->Size = 0;

		switch (resource->Type)
		{
		case ResourceType::VertexBuffer:
			VertexBuffer_D3D9* buffer = static_cast<VertexBuffer_D3D9*>(resource);
			delete buffer->Data;
			delete buffer;
			break;
		}
	}

	void VertexBuffer_D3D9::SetData(void* source, size_t offset, size_t size)
	{
		if (Handle == InvalidResourceHandle || Context == nullptr)
		{
			return;
		}

		if (offset + size > Size || !Dynamic)
		{
			return;
		}

		SDL_memcpy((u8*)Data + offset, source, size);

		IDirect3DVertexBuffer9* buffer = static_cast<IDirect3DVertexBuffer9*>(Context->BaseResource);
		UINT lockOffset = static_cast<UINT>(offset);
		UINT lockSize = static_cast<UINT>(size);

		void* bufferData = nullptr;
		buffer->Lock(lockOffset, lockSize, &bufferData, D3DLOCK_DISCARD);
		SDL_memcpy(bufferData, source, size);
		buffer->Unlock();
	}
}
#endif
