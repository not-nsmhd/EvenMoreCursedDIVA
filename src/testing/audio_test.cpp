#include <FAudio.h>
#include "audio_test.h"
#include "util/logging.h"
#include "global_res.h"
#include <fstream>

using namespace Logging;

namespace Testing
{
	static const int MAX_VOICES = 24;
	int lastVoiceUsed = 0;

	static GFX::Font* debugFont;

	static FAudio* faudio;
	static FAudioMasteringVoice* masterVoice;
	static u8* wavData = nullptr;
	static FAudioBuffer buffer = {};
	static FAudioVoice* sourceVoices[MAX_VOICES] = {};

	AudioTest* AudioTest::instance = new AudioTest();

	AudioTest* AudioTest::GetInstance()
	{
		return instance;
	}

	bool AudioTest::Initialize()
	{
		if (FAudioCreate(&faudio, 0, FAUDIO_DEFAULT_PROCESSOR) != 0)
		{
			return false;
		}

		if (FAudio_CreateMasteringVoice(faudio, &masterVoice, FAUDIO_DEFAULT_CHANNELS, FAUDIO_DEFAULT_SAMPLERATE, 0, 0, NULL) != 0)
		{
			return false;
		}

		return true;
	}
	
	bool AudioTest::LoadContent()
	{
		SDL_AudioSpec spec = {};
		u32 wavDataSize = 0;

		std::filesystem::path wavFilePath = fileSystem->GetContentFilePath("sounds/test_pcm.wav");
		SDL_LoadWAV(wavFilePath.generic_u8string().c_str(), &spec, &wavData, &wavDataSize);

		if (wavData == nullptr || wavDataSize == 0)
		{
			return false;
		}

		FAudioWaveFormatEx wfx = {};
		wfx.wFormatTag = FAUDIO_FORMAT_PCM;
		wfx.nChannels = spec.channels;
		wfx.nSamplesPerSec = spec.freq;
		wfx.wBitsPerSample = SDL_AUDIO_BITSIZE(spec.format);
		wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize = 0;

		for (int i = 0; i < MAX_VOICES; i++)
		{
			if (FAudio_CreateSourceVoice(faudio, &sourceVoices[i], &wfx, 0, 2.0f, NULL, NULL, NULL) != 0)
			{
				return false;
			}
		}

		buffer.pAudioData = wavData;
		buffer.AudioBytes = wavDataSize * 8;
		buffer.Flags = FAUDIO_END_OF_STREAM;

		debugFont = GlobalResources::DebugFont;
		spriteRenderer.Initialize(graphicsBackend);
		return true;
	}
	
	void AudioTest::UnloadContent()
	{
		spriteRenderer.Destroy();
		SDL_FreeWAV(wavData);
	}

	void AudioTest::Destroy()
	{
		for (int i = 0; i < MAX_VOICES; i++)
		{
			FAudioVoice_DestroyVoice(sourceVoices[i]);
		}

		FAudioVoice_DestroyVoice(masterVoice);
		FAudio_Release(faudio);
	}
	
	void AudioTest::OnResize(u32 newWidth, u32 newHeight)
	{
	}
	
	static void playSound()
	{
		if (lastVoiceUsed >= MAX_VOICES - 1)
		{
			lastVoiceUsed = 0;
		}

		FAudioVoiceState voiceState = {};
			
		for (int i = lastVoiceUsed; i < MAX_VOICES; i++)
		{
			FAudioSourceVoice_GetState(sourceVoices[i], &voiceState, 0);

			if (voiceState.BuffersQueued == 0)
			{
				FAudioSourceVoice_SubmitSourceBuffer(sourceVoices[i], &buffer, NULL);
				FAudioVoice_SetVolume(sourceVoices[i], 0.364f, FAUDIO_COMMIT_NOW);
				FAudioSourceVoice_Start(sourceVoices[i], 0, FAUDIO_COMMIT_NOW);
				return;
			}
		}
	}

	void AudioTest::Update()
	{
		if (keyboardState->IsKeyTapped(SDL_SCANCODE_SPACE))
		{
			playSound();
		}
	}
	
	void AudioTest::Draw()
	{
		graphicsBackend->Clear(GFX::LowLevel::ClearFlags::GFX_CLEAR_COLOR, Common::Color(0, 24, 24, 255), 1.0f, 0);

		graphicsBackend->SetBlendState(&GFX::LowLevel::DefaultBlendStates::AlphaBlend);
		debugFont->PushString(spriteRenderer, "Press 'Space' key to play test sound", glm::vec2(4.0f), glm::vec2(1.0f), Common::DefaultColors::White);
		spriteRenderer.RenderSprites(nullptr);

		graphicsBackend->SwapBuffers();
	}
}