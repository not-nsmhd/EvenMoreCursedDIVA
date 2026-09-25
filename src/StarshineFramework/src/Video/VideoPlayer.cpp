#include "VideoPlayer.h"
#include "Common/Logging/Logging.h"
#include <Rendering/D3D11/D3D11Device.h>
#include <Rendering/D3D11/D3D11Texture.h>
#include <mfapi.h>
#include <mfplay.h>
#include <mfidl.h>
#include <mfmediaengine.h>
#include <wrl.h>
#include <functional>
#include <utf8.h>

using namespace Starshine::Graphics;
using namespace Starshine::Rendering;
using namespace Starshine::Rendering::D3D11;
using namespace Microsoft::WRL;

namespace Starshine::Video
{
	constexpr const char* LogName = "Starshine::Video";

	class MediaEngineCallbackWrapper : public IMFMediaEngineNotify
	{
	public:
		MediaEngineCallbackWrapper(std::function<HRESULT(DWORD, DWORD_PTR, DWORD)> callback) : callbackFunc(callback), refCount(1) {};

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

	struct StrViewToBStr
	{
		StrViewToBStr(std::string_view strView)
		{
			std::unique_ptr<WCHAR[]> tempBuf = std::make_unique<WCHAR[]>(strView.size() + 1);
			utf8::utf8to16(strView.begin(), strView.end(), tempBuf.get());
			bstrData = ::SysAllocString(tempBuf.get());
		}

		~StrViewToBStr()
		{
			::SysFreeString(bstrData);
		}

		operator BSTR() const { return bstrData; }

		BSTR bstrData{};
	};

	class MFVideoPlayer : public VideoPlayer
	{
	public:
		MFVideoPlayer()
		{
		}

		~MFVideoPlayer()
		{
			Shutdown();
		}

	public:
		HRESULT Initialize()
		{
			if (initialized)
				return S_OK;

			HRESULT result{};

			gfxDevice = static_cast<D3D11Device*>(Rendering::GetDevice());
			ID3D11Device* d3dDev = gfxDevice->GetBaseDevice();
			ID3D11DeviceContext* d3dDevContext{};
			d3dDev->GetImmediateContext(&d3dDevContext);

			ID3D10Multithread* d3d10Multithread{};
			d3dDev->QueryInterface<ID3D10Multithread>(&d3d10Multithread);
			d3d10Multithread->SetMultithreadProtected(TRUE);

			if (result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET), result != S_OK)
			{
				LogError(LogName, "Failed to initialize Media Foundation. Error: 0x%08x", result);
				return result;
			}

			if (result = MFCreateDXGIDeviceManager(&MFDXGI.resetToken, &MFDXGI.devManager), result != S_OK)
			{
				LogError(LogName, "Failed to initialize MFDXGI device manager. Error: 0x%08x", result);
				return result;
			}

			if (result = d3dDevContext->QueryInterface<ID3D11VideoContext>(&MFDXGI.devVideoContext), result != S_OK)
			{
				LogError(LogName, "Failed to get D3D11 device's video context. Error: 0x%08x", result);
				return result;
			}

			MFDXGI.devManager->ResetDevice(d3dDev, MFDXGI.resetToken);

			callbackWrapper.Attach(new MediaEngineCallbackWrapper([&](DWORD event, DWORD_PTR param1, DWORD param2) -> HRESULT
				{
					return MENotifyCallback(event, param1, param2);
				}));

			if (result = CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&meClassFactory)), result != S_OK)
			{
				LogError(LogName, "Failed to create Media Engine class factory. Error: 0x%08x", result);
				return result;
			}

			MFCreateAttributes(&meInitAttribs, 3);
			meInitAttribs->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, MFDXGI.devManager.Get());
			meInitAttribs->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, callbackWrapper.Get());
			meInitAttribs->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, texOutputFormat);
			if (result = meClassFactory->CreateInstance(MF_MEDIA_ENGINE_REAL_TIME_MODE | MF_MEDIA_ENGINE_FORCEMUTE, meInitAttribs.Get(), &mediaEngine),
				result != S_OK)
			{
				LogError(LogName, "Failed to create Media Engine instance. Error: 0x%08x", result);
				return result;
			}

			mediaEngine->QueryInterface<IMFMediaEngineEx>(&mediaEngineEx);
			mediaEngine->SetMuted(TRUE);
			mediaEngine->SetVolume(0.0);
				
			initialized = true;
			return S_OK;
		}

		bool OpenFile(std::string_view filePath) override
		{
			HRESULT result = S_OK;
			if (result = mediaEngine->SetSource(StrViewToBStr(filePath)), result != S_OK)
			{
				LogError(LogName, "Failed to load video file \"%s\". Error: 0x%08x", filePath.data(), result);
				return false;
			}

			videoFileLoaded = true;
			return true;
		}

		void CloseFile() override
		{
			mediaEngine->Pause();
			mediaEngine->SetSource(nullptr);
			videoFileLoaded = false;
			canStart = false;
			playing = false;
		}

		bool GetIsPlaying() override
		{
			return playing;
		}

		void SetIsPlaying(bool playing) override
		{
			HRESULT result{};
			if (playing)
				result = mediaEngine->Play();
			else
				result = mediaEngine->Pause();

			this->playing = playing && (result == S_OK);
		}

		TimeSpan GetDuration() override
		{
			if (!videoFileLoaded)
				return {};
			return TimeSpanConversion::FromSeconds(videoDuration);
		}

		TimeSpan GetPosition() override
		{
			if (!videoFileLoaded)
				return {};
			return TimeSpanConversion::FromSeconds(currentVideoTime);
		}

		ivec2 GetResolution() override
		{
			if (!videoFileLoaded)
				return {};
			return ivec2(currentVideoSize.x, currentVideoSize.y);
		}

		Graphics::Texture* GetVideoTexture()
		{
			if (!canStart)
				return nullptr;

			LONGLONG presentTime{};
			if (mediaEngine->OnVideoStreamTick(&presentTime) == S_OK)
			{
				currentVideoTexBufferIndex = (currentVideoTexBufferIndex + 1 >= static_cast<i32>(videoTextureBuffer.size())) ? 0 : currentVideoTexBufferIndex + 1;
				D3D11Texture* d3dTex = static_cast<D3D11Texture*>(videoTextureBuffer[currentVideoTexBufferIndex]->GPUTexture.Resource.get());

				RECT transferRect{};
				transferRect.left = 0;
				transferRect.top = 0;
				transferRect.right = currentVideoSize.x;
				transferRect.bottom = currentVideoSize.y;
				mediaEngine->TransferVideoFrame(d3dTex->BaseTexture.Get(), nullptr, &transferRect, nullptr);

				currentVideoTime = mediaEngine->GetCurrentTime();
				return videoTextureBuffer[currentVideoTexBufferIndex].get();
			}
			else
			{
				return videoTextureBuffer[currentVideoTexBufferIndex].get();
			}

			return nullptr;
		}
	private:
		HRESULT MENotifyCallback(DWORD event, DWORD_PTR param1, DWORD param2)
		{
			switch (static_cast<MF_MEDIA_ENGINE_EVENT>(event))
			{
			case MF_MEDIA_ENGINE_EVENT_LOADSTART:
				break;
			case MF_MEDIA_ENGINE_EVENT_ERROR:
				LogError(LogName, "Media Engine errors 0x%08x 0x%08x", param1, param2);
				break;
			case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
				mediaEngine->GetNativeVideoSize(&currentVideoSize.x, &currentVideoSize.y);
				videoDuration = mediaEngine->GetDuration();
				CreateVideoTextures();
				break;
			case MF_MEDIA_ENGINE_EVENT_TIMEUPDATE:
				currentVideoTime = mediaEngine->GetCurrentTime();
				break;
			case MF_MEDIA_ENGINE_EVENT_CANPLAY:
				canStart = true;
				mediaEngine->Play();
				break;
			default:
				LogWarn(LogName, "Unhandled Media Engine event %d", event);
				break;
			}
			return S_OK;
		}

		void CreateVideoTextures()
		{
			for (size_t i = 0; i < videoTextureBuffer.size(); i++)
			{
				videoTextureBuffer[i] = std::make_unique<Graphics::Texture>(ivec2(currentVideoSize.x, currentVideoSize.y), TextureFormat::RGBA8, TextureFlags{});

				// HACK: hardware video decoding do be like that
				D3D11Device* renderingDevice = static_cast<D3D11Device*>(Rendering::GetDevice());

				D3D11_TEXTURE2D_DESC texDesc{};
				texDesc.Width = currentVideoSize.x;
				texDesc.Height = currentVideoSize.y;
				texDesc.MipLevels = 1;
				texDesc.ArraySize = 1;
				texDesc.Format = texOutputFormat;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.Usage = D3D11_USAGE_DEFAULT;
				texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

				videoTextureBuffer[i]->GPUTexture.Resource = nullptr;
				videoTextureBuffer[i]->GPUTexture.Resource = std::make_unique<D3D11Texture>(*renderingDevice, texDesc.Format, texDesc);
			}
		}

		void Shutdown()
		{
			for (auto& tex : videoTextureBuffer)
				tex = nullptr;

			mediaEngine->Shutdown();

			MFDXGI.devManager = nullptr;
			MFDXGI.devVideoContext = nullptr;

			mediaEngineEx = nullptr;
			mediaEngine = nullptr;
			callbackWrapper = nullptr;
			meClassFactory = nullptr;
			meInitAttribs = nullptr;

			MFShutdown();
		}
	private:
		D3D11Device* gfxDevice{};
		bool initialized{};

		static constexpr DXGI_FORMAT texOutputFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		ComPtr<IMFMediaEngineClassFactory> meClassFactory{};
		ComPtr<IMFAttributes> meInitAttribs{};
		ComPtr<IMFMediaEngine> mediaEngine{};
		ComPtr<IMFMediaEngineEx> mediaEngineEx{};
		ComPtr<MediaEngineCallbackWrapper> callbackWrapper{};

		struct
		{
			UINT resetToken{};
			ComPtr<IMFDXGIDeviceManager> devManager{};
			ComPtr<ID3D11VideoContext> devVideoContext{};
		} MFDXGI;

		glm::vec<2, DWORD> currentVideoSize{};
		bool videoFileLoaded{};
		bool canStart{};
		bool playing{};

		double currentVideoTime{};
		double videoDuration{};

		std::array<std::unique_ptr<Graphics::Texture>, 2> videoTextureBuffer{};
		i32 currentVideoTexBufferIndex{};
	};

	std::unique_ptr<VideoPlayer> InitializeVideoPlayer_MediaFoundation()
	{
		std::unique_ptr<MFVideoPlayer> videoPlayer = std::make_unique<MFVideoPlayer>();
		if (videoPlayer->Initialize() != S_OK)
			return nullptr;

		return videoPlayer;
	}
}
