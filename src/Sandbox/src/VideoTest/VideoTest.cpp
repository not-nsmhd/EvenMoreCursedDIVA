#include "VideoTestState.h"
#include "GameContext.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <evr.h>
#include <wmcodecdsp.h>
#include <Common/Logging/Logging.h>

using namespace Starshine;
using namespace Starshine::Rendering::Render2D;

namespace Sandbox::VideoTest
{
	struct VideoTestState::Impl
	{
		static constexpr const char* LogName = "Sandbox::VideoTest";

		VideoTestState& Parent;

		IMFMediaSession* mediaSession{};
		IMFSourceResolver* srcResolver{};

		MF_OBJECT_TYPE objectType{};
		IUnknown* object{};

		IMFMediaSource* mediaSource{};
		IMFAttributes* mediaAttribs{};
		IMFSourceReader* srcReader{};

		Impl(VideoTestState& parent) : Parent(parent) {}
		~Impl() {}

		bool InitMF()
		{
			HRESULT result = S_OK;
			if (result = MFStartup(MF_VERSION, MFSTARTUP_FULL), result != S_OK)
			{
				LogError(LogName, "MFStartup failed. Error: 0x%08x", result);
				return false;
			}

			if (result = MFCreateMediaSession(NULL, &mediaSession), result != S_OK)
			{
				LogError(LogName, "MFCreateMediaSession failed. Error: 0x%08x", result);
				return false;
			}

			if (result = MFCreateSourceResolver(&srcResolver), result != S_OK)
			{
				LogError(LogName, "MFCreateSourceResolver failed. Error: 0x%08x", result);
				return false;
			}

			srcResolver->CreateObjectFromURL(L"diva/videos/devtest01.mp4", MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_READ, NULL, &objectType, &object);
			object->QueryInterface(IID_PPV_ARGS(&mediaSource));
				
			return true;
		}

		void DestroyMF()
		{
			MFShutdown();
		}

		void DrawVideo(SpriteRenderer* sprRenderer)
		{
			
		}

		void Draw()
		{
			SpriteRenderer* sprRenderer = GameContext::GetInstance()->SpriteRenderer.get();
			auto& debugFont = GameContext::GetInstance()->DebugFont;

			DrawVideo(sprRenderer);

			sprRenderer->GetRenderingDevice()->Clear(Rendering::ClearFlags_Color, DefaultColors::ClearColor_InGame, 1.0f, 0);

			sprRenderer->Font().PushString(debugFont.get(), "jriofjosijfoej video test", vec2(0.0f), vec2(1.0f), DefaultColors::White);
			sprRenderer->RenderSprites(nullptr);
		}
	};

	VideoTestState::VideoTestState() : impl(std::make_unique<Impl>(*this))
	{
	}

	VideoTestState::~VideoTestState()
	{
	}

	bool VideoTestState::Initialize()
	{
		return impl->InitMF();
	}

	bool VideoTestState::LoadContent()
	{
		return true;
	}

	void VideoTestState::UnloadContent()
	{
	}

	void VideoTestState::Destroy()
	{
		impl->DestroyMF();
	}

	void VideoTestState::Update(Starshine::GameTime& gameTime)
	{
	}

	void VideoTestState::Draw(Starshine::GameTime& gameTime)
	{
		impl->Draw();
	}
}
