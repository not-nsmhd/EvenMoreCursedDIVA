#include "AudioTest.h"
#include "common/color.h"
#include "common/math_ext.h"
#include "gfx/Renderer.h"
#include "gfx/Render2D/SpriteRenderer.h"
#include "audio/AudioEngine.h"
#include "audio/SampleProvider/MemorySampleProvider.h"
#include "audio/SampleProvider/StreamingSampleProvider.h"
#include "input/Keyboard.h"
#include "io/File.h"
#include "util/logging.h"

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

		StreamingSampleProvider* sampleProvider;
		SourceHandle source = InvalidSourceHandle;

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

			/*SDL_RWops* testWavFile = SDL_RWFromFile("diva/sounds/test_wav_mono.wav", "rb");
			SDL_AudioSpec wavSpec = {};

			u8* wavData = nullptr;
			u32 wavSize = 0;

			SDL_LoadWAV_RW(testWavFile, 1, &wavSpec, &wavData, &wavSize);

			sampleProvider.sampleCount = static_cast<size_t>(wavSize) / sizeof(i16);
			sampleProvider.samples = new i16[sampleProvider.sampleCount];
			sampleProvider.channelCount = wavSpec.channels;
			sampleProvider.sampleRate = wavSpec.freq;

			SDL_memcpy(sampleProvider.samples, wavData, sampleProvider.sampleCount * sizeof(i16));

			SDL_FreeWAV(wavData);

			source = AudioEngine->RegisterSource(&sampleProvider);*/

			// -------------
			/*OggVorbis_File oggFile{};
			ov_fopen("diva/sounds/test_vorbis.ogg", &oggFile);

			vorbis_info* info = ov_info(&oggFile, -1);
			vorbis_comment* comments = ov_comment(&oggFile, -1);
			for (int i = 0; i < comments->comments; i++)
			{
				Logging::LogInfo("VorbisTest", "Comment %d: %s", i, comments->user_comments[i]);
			}

			sampleProvider.sampleCount = static_cast<size_t>(ov_pcm_total(&oggFile, -1) * info->channels);
			sampleProvider.samples = new i16[sampleProvider.sampleCount];
			sampleProvider.channelCount = info->channels;
			sampleProvider.sampleRate = info->rate;

			int currentSection = 0;
			char oggBuffer[4096] = {};
			size_t copyOffset = 0;

			while (true)
			{
				long bytesRead = ov_read(&oggFile, oggBuffer, sizeof(oggBuffer), 0, 2, 1, &currentSection);
				if (bytesRead == 0 || copyOffset >= sampleProvider.sampleCount * static_cast<size_t>(sampleProvider.channelCount))
				{
					break;
				}

				// NOTE: Gotta love C++'s pointer arithmetic and trying to make it libraries which work with raw bytes specifically
				// https://stackoverflow.com/a/394777
				SDL_memcpy(reinterpret_cast<char*>(sampleProvider.samples) + copyOffset, oggBuffer, static_cast<size_t>(bytesRead));
				copyOffset += static_cast<size_t>(bytesRead);
			}

			ov_clear(&oggFile);
			source = AudioEngine->RegisterSource(&sampleProvider);*/
			
			// -------------

			source = AudioEngine->LoadSourceFromFile("diva/sounds/test_wav_mono.wav");

			u8* oggData = nullptr;
			size_t oggSize = File::ReadAllBytes("diva/music/test.ogg", &oggData);

			sampleProvider = new StreamingSampleProvider(oggSize, oggData);

			SDL_RWops* outputDataFile = SDL_RWFromFile("diva/music/test_decoded.dat", "wb");
			i16 outputBuffer[2048] = {};

			while (true)
			{
				size_t decodedAmount = sampleProvider->GetNextSamples(outputBuffer, 2048);
				if (decodedAmount == 0)
				{
					break;
				}

				SDL_RWwrite(outputDataFile, outputBuffer, sizeof(i16), decodedAmount);
			}

			SDL_RWclose(outputDataFile);

			return true;
		}

		void Destroy()
		{
			AudioEngine->FreeLoadedSource(source);

			TestFont.Destroy();
			SpriteRenderer->Destroy();
		}

		void Update(f64 deltaTime_milliseconds)
		{
			if (Keyboard::IsKeyTapped(SDLK_SPACE))
			{
				AudioEngine->PlayOneShotSound(source);
			}
		}

		void Draw(f64 deltaTime_milliseconds)
		{
			BaseRenderer->Clear(ClearFlags_Color, Color(0, 24, 24, 255), 1.0f, 0);

			//char debug_voicePos[64]{};
			//SDL_snprintf(debug_voicePos, sizeof(debug_voicePos) - 1, "\nVoice Position: %lld/%lld", testVoice.GetSamplePosition(), audioDataSize);

			SpriteRenderer->Font().PushString(TestFont, "Audio Test", vec2(0.0f), vec2(1.0f), DefaultColors::White);
			SpriteRenderer->Font().PushString(TestFont, "\nPress space to play a sound", vec2(0.0f), vec2(1.0f), DefaultColors::White);
			//SpriteRenderer->Font().PushString(TestFont, debug_voicePos, vec2(0.0f), vec2(1.0f), DefaultColors::White);
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
