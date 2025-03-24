#include "audio.h"
#include "audio_defs.h"

namespace Audio
{
	struct AudioVoice
	{
		FAudioVoice* faudioVoice = nullptr;
		u32 queuedBuffers = 0;
		bool active = false;
	};

	AudioEngine* AudioEngine::instance = nullptr;

	AudioEngine* AudioEngine::GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new AudioEngine();
		}

		return instance;
	}

	bool AudioEngine::Initialize()
	{
		if (FAudioCreate(&faudioBackend, 0, FAUDIO_DEFAULT_PROCESSOR) != 0)
		{
			LOG_ERROR("Failed to initialize FAudio backend");
			return false;
		}

		if (FAudio_CreateMasteringVoice(faudioBackend, &masteringVoice, FAUDIO_DEFAULT_CHANNELS, FAUDIO_DEFAULT_SAMPLERATE, 0, 0, NULL) != 0)
		{
			LOG_ERROR("Failed to create mastering voice");
			return false;
		}

		mainVoiceFormat.wFormatTag = FAUDIO_FORMAT_PCM;
		mainVoiceFormat.nChannels = 2;
		mainVoiceFormat.nSamplesPerSec = 44100;
		mainVoiceFormat.wBitsPerSample = 16;
		mainVoiceFormat.nBlockAlign = (2 * mainVoiceFormat.wBitsPerSample) / 8;
		mainVoiceFormat.nAvgBytesPerSec = mainVoiceFormat.nSamplesPerSec * mainVoiceFormat.nBlockAlign;
		mainVoiceFormat.cbSize = 0;

		voicesSFX = new AudioVoice[MAX_VOICES_SFX];
		for (int i = 0; i < MAX_VOICES_SFX; i++)
		{
			if (FAudio_CreateSourceVoice(faudioBackend, &voicesSFX[i].faudioVoice, &mainVoiceFormat, 0, 2.0f, NULL, NULL, NULL) != 0)
			{
				LOG_ERROR_ARGS("Failed to initialize SFX voice %d", i);
				return false;
			}
		}

		LOG_INFO("All SFX voices initialized");
		LOG_INFO_ARGS("Initialized FAudio backend (version: %02d.%02d.%02d)", FAUDIO_MAJOR_VERSION, FAUDIO_MINOR_VERSION, FAUDIO_PATCH_VERSION);

		initialized = true;
		return true;
	}
	
	void AudioEngine::Destroy()
	{
		for (int i = 0; i < MAX_VOICES_SFX; i++)
		{
			FAudioVoice_DestroyVoice(voicesSFX[i].faudioVoice);
		}

		delete[] voicesSFX;
		LOG_INFO("Destroyed all SFX voices");

		FAudioVoice_DestroyVoice(masteringVoice);
		FAudio_Release(faudioBackend);
		LOG_INFO("FAudio backend destroyed");
	}
	
	void AudioEngine::PlaySoundEffect(SoundEffect& soundEffect)
	{
		if (lastSFXvoiceUsed_index >= MAX_VOICES_SFX - 1)
		{
			lastSFXvoiceUsed_index = 0;
		}

		FAudioBuffer* bufferToSubmit = soundEffect.GetBuffer();

		if (bufferToSubmit == nullptr)
		{
			return;
		}

		FAudioVoiceState voiceState = {};
		AudioVoice* voice = nullptr;

		for (int i = lastSFXvoiceUsed_index; i < MAX_VOICES_SFX; i++)
		{
			voice = &voicesSFX[i];
			FAudioSourceVoice_GetState(voice->faudioVoice, &voiceState, 0);

			if (voiceState.BuffersQueued == 0)
			{
				FAudioSourceVoice_SubmitSourceBuffer(voice->faudioVoice, bufferToSubmit, NULL);
				FAudioVoice_SetVolume(voice->faudioVoice, soundEffect.Volume, FAUDIO_COMMIT_NOW);
				FAudioSourceVoice_Start(voice->faudioVoice, 0, FAUDIO_COMMIT_NOW);

				lastSFXvoiceUsed_index = i;
				return;
			}
		}
	}
	
	void AudioEngine::StopAllSFXVoices()
	{
		AudioVoice* voice = nullptr;

		for (int i = 0; i < MAX_VOICES_SFX; i++)
		{
			voice = &voicesSFX[i];
			
			FAudioSourceVoice_Stop(voice->faudioVoice, 0, FAUDIO_COMMIT_NOW);
			FAudioSourceVoice_FlushSourceBuffers(voice->faudioVoice);
		}
	}
}
