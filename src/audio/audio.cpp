#include "audio.h"
#include "audio_defs.h"

namespace Audio
{
	struct AudioVoice
	{
		FAudioVoice* faudioVoice = nullptr;
		u32 queuedBuffers = 0;
		bool active = false;

		Music* musicPtr = nullptr;
		FAudioVoiceCallback callbacks = {};
	};

	namespace FAudioCallbacks
	{
		u8* DecodeNextMusicBuffer(AudioVoice* voice, size_t* decodedSize, int* lastBuffer)
		{
			if (voice == nullptr)
			{
				return nullptr;
			}

			if (voice->musicPtr == nullptr)
			{
				return nullptr;
			}

			return voice->musicPtr->DecodeNextPCMBlock(decodedSize, lastBuffer);
		}

		void DecodeNextMusicBuffer_Callback(FAudioVoiceCallback* callback, void* voicePtr)
		{
			if (voicePtr == nullptr)
			{
				return;
			}

			AudioVoice* voice = static_cast<AudioVoice*>(voicePtr);

			size_t decodedSize = 0;
			int lastBuffer = 0;
			u8* decodedData = DecodeNextMusicBuffer(voice, &decodedSize, &lastBuffer);

			if (decodedData != nullptr)
			{
				FAudioBuffer pcmDataBuffer = {};
				pcmDataBuffer.AudioBytes = decodedSize;
				pcmDataBuffer.pAudioData = decodedData;
				pcmDataBuffer.Flags = (lastBuffer == 1 ? FAUDIO_END_OF_STREAM : 0);
				pcmDataBuffer.pContext = voicePtr;

				FAudioSourceVoice_SubmitSourceBuffer(voice->faudioVoice, &pcmDataBuffer, NULL);
			}
		}
	}

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
		
		voicesStreaming = new AudioVoice[MAX_VOICES_STREAMING];
		for (int i = 0; i < MAX_VOICES_STREAMING; i++)
		{
			voicesStreaming[i].callbacks.OnBufferEnd = (&FAudioCallbacks::DecodeNextMusicBuffer_Callback);

			if (FAudio_CreateSourceVoice(faudioBackend, &voicesStreaming[i].faudioVoice, &mainVoiceFormat, 0, 2.0f, &voicesStreaming[i].callbacks, NULL, NULL) != 0)
			{
				LOG_ERROR_ARGS("Failed to initialize streaming voice %d", i);
				return false;
			}
		}
		LOG_INFO("All streaming voices initialized");

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

		for (int i = 0; i < MAX_VOICES_STREAMING; i++)
		{
			FAudioVoice_DestroyVoice(voicesStreaming[i].faudioVoice);
		}

		delete[] voicesStreaming;
		LOG_INFO("Destroyed all streaming voices");

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
	
	void AudioEngine::PlayMusic(Music* music, u32 voiceIndex)
	{
		if (voiceIndex >= MAX_VOICES_STREAMING - 1)
		{
			return;
		}

		AudioVoice* voice = nullptr;

		voice = &voicesStreaming[voiceIndex];
		if (!voice->active && voice->musicPtr == nullptr)
		{
			voice->musicPtr = music;

			FAudioCallbacks::DecodeNextMusicBuffer_Callback(nullptr, voice);
			FAudioVoice_SetVolume(voice->faudioVoice, 0.7f, FAUDIO_COMMIT_NOW);
			FAudioSourceVoice_Start(voice->faudioVoice, 0, FAUDIO_COMMIT_NOW);
			voice->active = true;

			return;
		}
	}

	void AudioEngine::PauseStreamingVoice(u32 index)
	{
		AudioVoice* voice = nullptr;
		voice = &voicesStreaming[index];

		if (voice->active && voice->musicPtr != nullptr)
		{
			FAudioSourceVoice_Stop(voice->faudioVoice, 0, FAUDIO_COMMIT_NOW);
		}
	}

	void AudioEngine::ResumeStreamingVoice(u32 index)
	{
		AudioVoice* voice = nullptr;
		voice = &voicesStreaming[index];

		if (voice->active && voice->musicPtr != nullptr)
		{
			FAudioSourceVoice_Start(voice->faudioVoice, 0, FAUDIO_COMMIT_NOW);
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
	
	void AudioEngine::StopAllStreamingVoices()
	{
		AudioVoice* voice = nullptr;

		for (int i = 0; i < MAX_VOICES_STREAMING; i++)
		{
			voice = &voicesStreaming[i];
			
			if (voice->active && voice->musicPtr != nullptr)
			{
				FAudioSourceVoice_Stop(voice->faudioVoice, 0, FAUDIO_COMMIT_NOW);
				FAudioSourceVoice_FlushSourceBuffers(voice->faudioVoice);

				voice->active = false;
				voice->musicPtr = nullptr;
			}
		}
	}
}
