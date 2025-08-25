#include "AudioTest.h"
#include "common/color.h"
#include "common/math_ext.h"
#include "gfx/Renderer.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "audio/AudioEngine.h"
#include "input/Keyboard.h"
#include "io/File.h"

namespace Starshine::Testing
{
	using namespace Starshine::GFX;
	using namespace Starshine::GFX::Render2D;
	using namespace Starshine::Audio;
	using namespace Starshine::Input;
	using namespace Starshine::IO;
	using namespace Common;
	using std::string;
	using std::string_view;

	struct AudioTest::Impl
	{
		Renderer* BaseRenderer = nullptr;
		AudioEngine* AudioEngine = nullptr;

		SpriteRenderer* SpriteRenderer = nullptr;
		Font TestFont;

		i16* audioData = nullptr;
		size_t audioDataSize = 0;
		SourceHandle source = InvalidSourceHandle;
		Voice testVoice = {};

		bool Initialize()
		{
			BaseRenderer = Renderer::GetInstance();
			AudioEngine = AudioEngine::GetInstance();
			return true;
		}

		bool LoadContent()
		{
			SpriteRenderer = new Render2D::SpriteRenderer();

			TestFont.ReadBMFont("diva/fonts/debug.fnt");

			SDL_RWops* testWavFile = SDL_RWFromFile("diva/sounds/test_wav.wav", "rb");
			SDL_AudioSpec wavSpec = {};

			u8* wavData = nullptr;
			u32 wavSize = 0;

			SDL_LoadWAV_RW(testWavFile, 1, &wavSpec, &wavData, &wavSize);
			audioDataSize = static_cast<size_t>(wavSize) / sizeof(i16);
			audioData = new i16[audioDataSize];
			SDL_memcpy(audioData, wavData, audioDataSize * 2);

			SDL_FreeWAV(wavData);

			source = AudioEngine->RegisterSource(audioData, audioDataSize);
			VoiceHandle voiceHandle = AudioEngine->AllocateVoice(source);
			testVoice = Voice(voiceHandle);

			return true;
		}

		void Destroy()
		{
			AudioEngine->FreeVoice(testVoice.GetHandle());
			AudioEngine->FreeSource(source);

			delete[] audioData;
			TestFont.Destroy();
			SpriteRenderer->Destroy();
		}

		void Update(f64 deltaTime_milliseconds)
		{
			if (Keyboard::IsKeyTapped(SDLK_SPACE))
			{
				testVoice.SetIsPlaying(true);
			}
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			BaseRenderer->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);

			char debug_voicePos[64]{};
			SDL_snprintf(debug_voicePos, sizeof(debug_voicePos) - 1, "\nVoice Position: %lld/%lld", testVoice.GetSamplePosition(), audioDataSize);

			SpriteRenderer->Font().PushString(TestFont, "Audio Test", vec2(0.0f), vec2(1.0f), DefaultColors::White);
			SpriteRenderer->Font().PushString(TestFont, debug_voicePos, vec2(0.0f), vec2(1.0f), DefaultColors::White);
			SpriteRenderer->RenderSprites(nullptr);

			BaseRenderer->SwapBuffers();
		}
	};

	AudioTest::AudioTest() : impl(new AudioTest::Impl())
	{
	}

	bool AudioTest::Initialize()
	{
		return impl->Initialize();
	}

	bool AudioTest::LoadContent()
	{
		return impl->LoadContent();
	}

	void AudioTest::UnloadContent()
	{
	}

	void AudioTest::Destroy()
	{
		impl->Destroy();
		delete impl;
	}

	void AudioTest::Update(f64 deltaTime_milliseconds)
	{
		impl->Update(deltaTime_milliseconds);
	}

	void AudioTest::Draw(f64 deltaTime_milliseconds)
	{
		impl->Draw(deltaTime_milliseconds);
	}

	string_view AudioTest::GetStateName() const
	{
		return "[Dev] Audio Test";
	}
}
