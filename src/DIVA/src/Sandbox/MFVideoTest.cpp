#include "MFVideoTest.h"
#include <Common/Logging/Logging.h>
#include "../GameContext.h"
#include <Graphics/Texture.h>
#include <Rendering/D3D11/D3D11Device.h>
#include <Rendering/D3D11/D3D11Texture.h>
#include <Input/Keyboard.h>

#include <mfapi.h>
#include <mfplay.h>
#include <mfidl.h>
#include <mfmediaengine.h>
#include <functional>
#include <wrl.h>

using namespace Starshine;
using namespace Starshine::Graphics;
using namespace Starshine::Rendering;
using namespace Starshine::Rendering::D3D11;
using namespace Starshine::Rendering::Render2D;
using namespace Starshine::Input;
using namespace Microsoft::WRL;

namespace DIVA::Sandbox
{
	class MediaEngineNotify : public IMFMediaEngineNotify
	{
	public:
		MediaEngineNotify(std::function<HRESULT(DWORD, DWORD_PTR, DWORD)> callback) : callbackFunc(callback), refCount(1) {};

	public:
		STDMETHODIMP_(ULONG) AddRef()
		{
			return InterlockedIncrement(&refCount);
		}

		STDMETHODIMP_(ULONG) Release()
		{
			LONG remainingRefs = InterlockedDecrement(&refCount);
			if (remainingRefs == 0)
				delete this;

			return remainingRefs;
		}

		STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
		{
			if (iid == IID_IMFMediaEngineNotify)
				*ppv = static_cast<IMFMediaEngineNotify*>(this);
			else
			{
				*ppv = nullptr;
				return E_NOINTERFACE;
			}

			AddRef();
			return S_OK;
		}

	public:
		STDMETHODIMP EventNotify(DWORD event, DWORD_PTR param1, DWORD param2)
		{
			return callbackFunc(event, param1, param2);
		}

	private:
		ULONG refCount{};
		std::function<HRESULT(DWORD, DWORD_PTR, DWORD)> callbackFunc{};
	};

	struct MFVideoTest::Impl
	{
		struct MFContext
		{
			ComPtr<IMFMediaEngineClassFactory> meClassFactory{};
			ComPtr<IMFAttributes> meAttribs{};
			ComPtr<IMFMediaEngine> mediaEngine{};
			ComPtr<IMFMediaEngineEx> mediaEngineEx{};
			ComPtr<MediaEngineNotify> meNotify{};
		} MF;

		struct MFDXGIContext
		{
			UINT resetToken{};
			ComPtr<IMFDXGIDeviceManager> devManager{};

			ComPtr<ID3D11VideoContext> deviceVideoContext{};

			DWORD videoWidth{};
			DWORD videoHeight{};
		} MFDXGI;

		std::array<std::unique_ptr<Graphics::Texture>, 2> videoTextures{};
		i32 videoTextureToDisplay{};

		bool loaded = false;
		f64 currentTime{};

		Impl()
		{
		}

		HRESULT MENotifyCallback(DWORD event, DWORD_PTR param1, DWORD param2)
		{
			switch (static_cast<MF_MEDIA_ENGINE_EVENT>(event))
			{
			case MF_MEDIA_ENGINE_EVENT_LOADSTART:
				LogMessage("Loading started");
				break;
			case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
				MF.mediaEngine->GetNativeVideoSize(&MFDXGI.videoWidth, &MFDXGI.videoHeight);
				CreateVideoTexture();
				break;
			case MF_MEDIA_ENGINE_EVENT_CANPLAY:
				LogMessage("Can play");
				MF.mediaEngine->Play();
				break;
			case MF_MEDIA_ENGINE_EVENT_TIMEUPDATE:
				currentTime = MF.mediaEngine->GetCurrentTime();
				break;
			default:
				LogMessage("Callback MF engine event %d", event);
				break;
			}
			return S_OK;
		}

		void CreateVideoTexture()
		{
			TextureFlags flags{};
			for (size_t i = 0; i < videoTextures.size(); i++)
			{
				videoTextures[i] = std::make_unique<Graphics::Texture>(ivec2(MFDXGI.videoWidth, MFDXGI.videoHeight), TextureFormat::RGBA8, TextureFlags{});

				// HACK: hardware video decoding do be like that
				D3D11Device* renderingDevice = static_cast<D3D11Device*>(Rendering::GetDevice());

				D3D11_TEXTURE2D_DESC texDesc{};
				texDesc.Width = MFDXGI.videoWidth;
				texDesc.Height = MFDXGI.videoHeight;
				texDesc.MipLevels = 1;
				texDesc.ArraySize = 1;
				texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.Usage = D3D11_USAGE_DEFAULT;
				texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

				videoTextures[i]->GPUTexture.Resource = nullptr;
				videoTextures[i]->GPUTexture.Resource = std::make_unique<D3D11Texture>(*renderingDevice, texDesc.Format, texDesc);
			}
		}

		bool Initialize()
		{
			HRESULT result{};

			D3D11Device* renderingDevice = static_cast<D3D11Device*>(Rendering::GetDevice());
			ID3D11Device* baseDevice = renderingDevice->GetBaseDevice();

			ID3D11DeviceContext* devContext{};
			baseDevice->GetImmediateContext(&devContext);

			ID3D10Multithread* d3d10Multithread{};
			baseDevice->QueryInterface<ID3D10Multithread>(&d3d10Multithread);
			d3d10Multithread->SetMultithreadProtected(TRUE);

			MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
			MFCreateDXGIDeviceManager(&MFDXGI.resetToken, &MFDXGI.devManager);
			devContext->QueryInterface<ID3D11VideoContext>(&MFDXGI.deviceVideoContext);
			MFDXGI.devManager->ResetDevice(baseDevice, MFDXGI.resetToken);

			MF.meNotify.Attach(new MediaEngineNotify([&](DWORD event, DWORD_PTR param1, DWORD param2) -> HRESULT
				{
					return MENotifyCallback(event, param1, param2);
				}));

			if (result = CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&MF.meClassFactory)), result != S_OK)
				return false;

			MFCreateAttributes(&MF.meAttribs, 3);
			MF.meAttribs->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, (IUnknown*)MFDXGI.devManager.Get());
			MF.meAttribs->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, (IUnknown*)MF.meNotify.Get());
			MF.meAttribs->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, DXGI_FORMAT_R8G8B8A8_UNORM);
			MF.meClassFactory->CreateInstance(MF_MEDIA_ENGINE_REAL_TIME_MODE, MF.meAttribs.Get(), &MF.mediaEngine);

			MF.mediaEngine->QueryInterface<IMFMediaEngineEx>(&MF.mediaEngineEx);
			MF.mediaEngine->SetMuted(TRUE);
			MF.mediaEngine->SetVolume(0.0);
			
			return true;
		}

		bool LoadContent()
		{
			BSTR filePath = SysAllocString(L"diva/videos/devtest_long.mp4");

			HRESULT result = S_OK;
			if (result = MF.mediaEngine->SetSource(filePath), result != S_OK)
			{
				LogMessage("Failed to load test video file. Error: 0x%08x", result);
				return false;
			}

			SysFreeString(filePath);
			return true;
		}

		void Destroy()
		{
			videoTextures[0] = nullptr;
			videoTextures[1] = nullptr;
			MF.mediaEngine->Shutdown();
		}

		void Update(GameTime& gameTime)
		{
			LONGLONG pts{};
			if (MF.mediaEngine->OnVideoStreamTick(&pts) == S_OK)
			{
				videoTextureToDisplay = (videoTextureToDisplay + 1 >= static_cast<i32>(videoTextures.size())) ? 0 : videoTextureToDisplay + 1;
				D3D11Texture* d3dTex = static_cast<D3D11Texture*>(videoTextures[videoTextureToDisplay]->GPUTexture.Resource.get());

				RECT transferRect{};
				transferRect.left = 0;
				transferRect.top = 0;
				transferRect.right = MFDXGI.videoWidth;
				transferRect.bottom = MFDXGI.videoHeight;
				MF.mediaEngine->TransferVideoFrame(d3dTex->BaseTexture.Get(), nullptr, &transferRect, nullptr);

				currentTime = MF.mediaEngine->GetCurrentTime();
			}
		}

		void Draw()
		{
			using namespace Starshine::Rendering;

			auto gfxDevice = Rendering::GetDevice();
			auto sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();

			auto& fontRenderer = sprRenderer->Font();
			auto font = GameContext::GetInstance()->DebugFont.get();

			gfxDevice->Clear(ClearFlags_Color, DefaultColors::Gray, 1.0f, 0);

			Graphics::Texture* videoTex = videoTextures[videoTextureToDisplay].get();
			sprRenderer->SetBlendMode(BlendMode::Normal);
			sprRenderer->SetSpritePosition(vec2(0.0f));
			sprRenderer->SetSpriteSize(vec2(1280.0f, 720.0f));
			sprRenderer->SetSpriteSource(RectangleF(0.0f, 0.0f, 1.0f, 1.0f));
			sprRenderer->SetSpriteColor(DefaultColors::White);
			sprRenderer->PushSprite(videoTex);

			char buffer[128]{};
			SDL_snprintf(buffer, sizeof(buffer), "Current time: %0.3f", currentTime);

			fontRenderer.PushString(font, buffer, vec2(0.0f), vec2(1.0f), DefaultColors::White, DefaultColors::Black);

			sprRenderer->Present();
		}
	};

	MFVideoTest::MFVideoTest()
	{
	}

	MFVideoTest::~MFVideoTest()
	{
	}

	bool MFVideoTest::Initialize()
	{
		GameInstance->GetWindow()->SetTitle("Testing");
		impl = std::make_unique<Impl>();
		return impl->Initialize();
	}

	bool MFVideoTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void MFVideoTest::UnloadContent()
	{
	}

	void MFVideoTest::Destroy()
	{
		GameInstance->GetWindow()->SetTitle("Even More Cursed DIVA");
		impl->Destroy();
		impl = nullptr;
	}

	void MFVideoTest::Update(Starshine::GameTime& gameTime)
	{
		impl->Update(gameTime);

		if (Keyboard::IsKeyTapped(SDLK_ESCAPE))
			GameInstance->Quit();
	}

	void MFVideoTest::Draw(Starshine::GameTime& gameTime)
	{
		impl->Draw();
	}
}

