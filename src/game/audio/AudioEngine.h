#pragma once
#include "common/types.h"
#include "SampleProvider/ISampleProvider.h"

namespace Starshine::Audio
{
	// NOTE: Sample: Raw 16-bit PCM data for a single channel
	// NOTE: Frame: A set of samples spanning across each channel
	// NOTE: Mixing Sample: 32-bit floating point representation of a 16-bit PCM sample

	enum class VoiceHandle : u16 { Invalid = 0xFFFF };
	enum class SourceHandle : u16 { Invalid = 0xFFFF };

	class Voice
	{
	public:
		Voice() : Handle(VoiceHandle::Invalid) {}
		Voice(VoiceHandle handle) : Handle(handle) {}

		VoiceHandle Handle{};

		inline operator VoiceHandle() const { return Handle; }

	public:
		bool IsValid() const;

		bool IsPlaying() const;
		void SetPlaying(bool play);

		bool IsLooped() const;
		void SetLoopState(bool loop);

		SourceHandle GetSource() const;
		void SetSource(SourceHandle handle);

		size_t GetFramePosition() const;
		void SetFramePosition(size_t position);
	};

	class AudioEngine : NonCopyable
	{
		friend Voice;

	public:
		AudioEngine();
		~AudioEngine();

	public:
		static constexpr u8 DefaultChannelCount = 2;
		static constexpr i32 DefaultSampleRate = 44100;
		static constexpr u16 DefaultSampleBufferSize = 2048;

		static constexpr size_t MaxSimultaneousVoices = 128;

	public:
		static void CreateInstance();
		static void DestroyInstance();

		static AudioEngine* GetInstance();

	public:
		bool Initialize();
		void Destroy();

	public:
		// NOTE: This function makes a local copy of the "samples" array that is stored in the source context
		SourceHandle RegisterSource(ISampleProvider* sampleProvider);
		SourceHandle LoadSource(const void* encodedData, size_t encodedDataSize);
		SourceHandle LoadSource(std::string_view filePath);
		void UnloadSource(SourceHandle handle);

		VoiceHandle AllocateVoice(SourceHandle source);
		void FreeVoice(VoiceHandle handle);

	public:
		// NOTE: This function is meant to be used only as a callback for SDL's audio subsystem.
		// 'length' is the amount of samples in the 'stream' array.
		void QueueAudioCallback(f32* stream, size_t length);

	private:
		struct Impl;
		Impl* impl = nullptr;
	};
}
