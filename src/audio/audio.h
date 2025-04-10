#pragma once
#include <SDL2/SDL.h>
#include <FAudio.h>
#include "../common/int_types.h"
#include "sound_effect.h"
#include "music.h"

namespace Audio
{
	struct AudioVoice;

	class AudioEngine
	{
	protected:
		AudioEngine() {};
		static AudioEngine* instance;

	public:
		AudioEngine(AudioEngine& other) = delete;
		void operator=(const AudioEngine&) = delete;

		static AudioEngine* GetInstance();

		bool Initialize();
		void Destroy();

		void PlaySoundEffect(SoundEffect& soundEffect);
		void PlayMusic(Music* music);

		void StopAllSFXVoices();
		void StopAllStreamingVoices();
	private:
		const u32 MAX_VOICES_SFX = 48;
		const u32 MAX_VOICES_STREAMING = 4;

		bool initialized = false;

		FAudio* faudioBackend = nullptr;
		FAudioMasteringVoice* masteringVoice = nullptr;
		FAudioWaveFormatEx mainVoiceFormat = {};

		AudioVoice* voicesSFX = nullptr;
		size_t lastSFXvoiceUsed_index = 0;

		AudioVoice* voicesStreaming = nullptr;
		size_t lastStreamingVoiceUsed_index = 0;
	};
}
