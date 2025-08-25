#pragma once
#include "common/types.h"
#include "SampleProvider/ISampleProvider.h"

namespace Starshine::Audio
{
	// NOTE: Voice Handle: Index into an array of voices
	// NOTE: Voice Context: Implementation-specifc context for voices
	// NOTE: Sample: Single signed 16-bit PCM
	// NOTE: Frame: A pair of samples for each channel

	using VoiceHandle = u16;
	using SourceHandle = u16;
	constexpr VoiceHandle InvalidVoiceHandle = 0xFFFF;
	constexpr SourceHandle InvalidSourceHandle = 0xFFFF;

	struct Voice
	{
	public:
		Voice() : Handle(InvalidVoiceHandle) {}
		Voice(VoiceHandle handle) : Handle(handle) {}

	public:
		bool GetIsPlaying() const;
		void SetIsPlaying(bool playing);

		size_t GetSamplePosition() const;
		void SetSamplePosition(size_t position);

		VoiceHandle GetHandle() const;

	private:
		VoiceHandle Handle{ InvalidVoiceHandle };

		bool GetInternalFlag(u16 flag) const;
		void SetInternalFlag(u16 flag, bool value);
	};

	namespace VoiceCallbacks
	{
		void OnBufferEnd(void* context);
	}

	class AudioEngine : NonCopyable
	{
		friend Voice;

	public:
		AudioEngine();
		~AudioEngine() = default;

	public:
		static constexpr u32 DefaultChannels = 2;
		static constexpr u32 DefaultSampleRate = 44100;
		static constexpr u32 DefaultBitsPerSample = 16;

		static constexpr size_t MaxVoices = 128;

	public:
		static void CreateInstance();
		static void DeleteInstance();

		static AudioEngine* GetInstance();

	public:
		void UpdateVoices();

	public:
		// NOTE: Registers a source consisting of raw 16-bit PCM data in a WAV file
		// 'data' is the array of PCM samples. Pointer must be valid until 'FreeSource' is called
		// 'size' is the amount of samples, not bytes
		SourceHandle RegisterSource(ISampleProvider* sampleProvider);
		void FreeSource(SourceHandle handle);

		VoiceHandle AllocateVoice(SourceHandle source);
		void FreeVoice(VoiceHandle voice);

		void PlayOneShotSound(SourceHandle source);

	public:
		ISampleProvider* GetSource(SourceHandle handle);

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
