#include "AudioEngine.h"
#include <FAudio.h>
#include <SDL2/SDL.h>
#include <array>
#include <vector>
#include "util/logging.h"

namespace Starshine::Audio
{
	using namespace Logging;
	using std::array;
	using std::vector;

	constexpr const char* LogName = "Starshine::Audio::AudioEngine";
	constexpr size_t DefaultBufferedSampleAmount = 2048;

	AudioEngine* GlobalInstance = nullptr;

	struct SampleProvider
	{
		i16* Samples = nullptr;
		size_t SampleAmount = 0;

		size_t GetSamples(i16* bufferToFill, size_t sampleOffset, size_t sampleCount)
		{
			assert(bufferToFill != nullptr);

			void* copySource = Samples + sampleOffset;
			size_t copyAmount = std::min(SampleAmount - sampleOffset, sampleCount);

			SDL_memcpy(bufferToFill, copySource, copyAmount * sizeof(i16));
			return copyAmount;
		}
	};

	enum VoiceStateFlags : u16
	{
		VoiceState_Free = 0,
		VoiceState_Allocated = 1 << 0,
		VoiceState_Playing = 1 << 1
	};

	struct VoiceContext
	{
		u16 VoiceFlags = 0;
		FAudioSourceVoice* SourceVoice = nullptr;
		bool StopOnNextBuffer = false;

		SourceHandle Source = InvalidSourceHandle;
		size_t SamplePosition = 0;

		i16 BufferData[DefaultBufferedSampleAmount] = {};

		void SubmitNextBuffer(size_t sampleCount, bool lastBuffer)
		{
			if (StopOnNextBuffer)
			{
				VoiceFlags &= ~VoiceState_Playing;
				SamplePosition = 0;

				FAudioSourceVoice_Stop(SourceVoice, 0, FAUDIO_COMMIT_NOW);

				StopOnNextBuffer = false;
				return;
			}

			FAudioBuffer buffer = {};

			buffer.pAudioData = reinterpret_cast<u8*>(BufferData);
			buffer.AudioBytes = static_cast<u32>(sampleCount * sizeof(i16));
			buffer.pContext = this;

			if (lastBuffer)
			{
				buffer.Flags |= FAUDIO_END_OF_STREAM;
				StopOnNextBuffer = true;
			}

			FAudioSourceVoice_SubmitSourceBuffer(SourceVoice, &buffer, NULL);
			SamplePosition += sampleCount;
		}
	};

#pragma region FAudio Callbacks
	namespace FAudioCallbacks
	{
		void OnBufferEnd(FAudioVoiceCallback* callback, void* context)
		{
			VoiceCallbacks::OnBufferEnd(context);
		}

		FAudioVoiceCallback CallbackData
		{
			OnBufferEnd,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		};
	}
#pragma endregion

#pragma region AudioEngine Internal Implementation
	struct AudioEngine::Impl
	{
		struct BackendData
		{
			FAudio* FAudio = nullptr;
			FAudioWaveFormatEx PlaybackFormat = {};
			FAudioMasteringVoice* MasteringVoice = nullptr;
		} Backend;

		array<VoiceContext, MaxVoices> VoiceContexts;
		vector<SampleProvider> RegisteredSources;

		Impl()
		{
			InitializeBackend();

			constexpr size_t initialSourceCapacity = 256;
			RegisteredSources.reserve(initialSourceCapacity);
		}

		void InitializeBackend()
		{
			u32 result = 0;
			if ((result = FAudioCreate(&Backend.FAudio, 0, FAUDIO_DEFAULT_PROCESSOR)) != 0)
			{
				LogError(LogName, "Failed to initialize FAudio. Error: 0x%08x", result);
				return;
			}

			u32 faudioVersion = FAudioLinkedVersion();
			LogInfo(LogName, "Linked FAudio Version: %d", faudioVersion);

			Backend.PlaybackFormat.wFormatTag = FAUDIO_FORMAT_PCM;
			Backend.PlaybackFormat.cbSize = 0;
			Backend.PlaybackFormat.nChannels = static_cast<u16>(DefaultChannels);
			Backend.PlaybackFormat.wBitsPerSample = static_cast<u16>(DefaultBitsPerSample);
			Backend.PlaybackFormat.nSamplesPerSec = DefaultSampleRate;
			Backend.PlaybackFormat.nBlockAlign = static_cast<u16>(DefaultChannels * DefaultBitsPerSample) / 8;

			if ((result = FAudio_CreateMasteringVoice(Backend.FAudio, &Backend.MasteringVoice, DefaultChannels, DefaultSampleRate, 0, 0, NULL)) != 0)
			{
				LogError(LogName, "Failed to create mastering voice. Error: 0x%08x", result);
				return;
			}
		}

		void Destroy()
		{
			FAudioVoice_DestroyVoice(Backend.MasteringVoice);
			FAudio_Release(Backend.FAudio);
		}

		FAudioSourceVoice* CreateFAudioVoice()
		{
			FAudioSourceVoice* sourceVoice = nullptr;
			FAudio_CreateSourceVoice(Backend.FAudio, &sourceVoice, &Backend.PlaybackFormat, 0, 2.0f, &FAudioCallbacks::CallbackData, NULL, NULL);

			return sourceVoice;
		}

		VoiceContext* GetVoiceContext(VoiceHandle handle)
		{
			if (handle == InvalidVoiceHandle || handle >= MaxVoices)
			{
				return nullptr;
			}

			size_t index = static_cast<size_t>(handle);
			auto& voiceCtx = VoiceContexts[index];

			if (voiceCtx.VoiceFlags == VoiceState_Free)
			{
				return nullptr;
			}

			return &voiceCtx;
		}

		SampleProvider* GetSource(SourceHandle handle)
		{
			if (handle == InvalidSourceHandle || handle >= RegisteredSources.size())
			{
				return nullptr;
			}

			size_t index = static_cast<size_t>(handle);
			return &RegisteredSources[index];
		}
	};
#pragma endregion

#pragma region Voice Callbacks Implementation
	void VoiceCallbacks::OnBufferEnd(void* context)
	{
		VoiceContext* voiceCtx = static_cast<VoiceContext*>(context);
		if ((voiceCtx->VoiceFlags & VoiceState_Playing) == 0)
		{
			return;
		}

		SampleProvider* source = GlobalInstance->GetSource(voiceCtx->Source);

		size_t decodedSamples = source->GetSamples(voiceCtx->BufferData, voiceCtx->SamplePosition, DefaultBufferedSampleAmount);
		bool lastBuffer = false;

		if (voiceCtx->SamplePosition + decodedSamples >= source->SampleAmount)
		{
			lastBuffer = true;
		}

		voiceCtx->SubmitNextBuffer(decodedSamples, lastBuffer);
	}
#pragma endregion

#pragma region AudioEngine Class Implementation
	AudioEngine::AudioEngine() : impl(new Impl())
	{
	}

	void AudioEngine::CreateInstance()
	{
		assert(GlobalInstance == nullptr);
		GlobalInstance = new AudioEngine();
	}

	void AudioEngine::DeleteInstance()
	{
		assert(GlobalInstance != nullptr);
		GlobalInstance->impl->Destroy();
		delete GlobalInstance->impl;
		delete GlobalInstance;
	}

	AudioEngine* AudioEngine::GetInstance()
	{
		assert(GlobalInstance != nullptr);
		return GlobalInstance;
	}

	SourceHandle AudioEngine::RegisterSource(i16* data, size_t size)
	{
		if (data == nullptr || size == 0)
		{
			return InvalidSourceHandle;
		}

		for (size_t i = 0; i < impl->RegisteredSources.size(); i++)
		{
			auto& sampleProvider = impl->RegisteredSources[i];

			if (sampleProvider.Samples != nullptr)
			{
				continue;
			}

			sampleProvider.Samples = data;
			sampleProvider.SampleAmount = size;

			return static_cast<SourceHandle>(i);
		}

		impl->RegisteredSources.push_back(SampleProvider { data, size });
		return static_cast<SourceHandle>(impl->RegisteredSources.size() - 1);
	}

	void AudioEngine::FreeSource(SourceHandle handle)
	{
		if (handle == InvalidSourceHandle || handle >= impl->RegisteredSources.size())
		{
			return;
		}

		auto& sampleProvider = impl->RegisteredSources[handle];
		sampleProvider.Samples = nullptr;
		sampleProvider.SampleAmount = 0;
	}

	VoiceHandle AudioEngine::AllocateVoice(SourceHandle source)
	{
		if (source == InvalidSourceHandle)
		{
			return InvalidVoiceHandle;
		}

		for (size_t i = 0; i < impl->VoiceContexts.size(); i++)
		{
			auto& voice = impl->VoiceContexts[i];
			if (voice.VoiceFlags != VoiceState_Free)
			{
				continue;
			}

			voice.Source = source;
			voice.SamplePosition = 0;
			voice.SourceVoice = impl->CreateFAudioVoice();

			voice.VoiceFlags = VoiceState_Allocated;

			LogInfo(LogName, "Voice %lld has been allocated", i);
			return static_cast<VoiceHandle>(i);
		}

		return InvalidVoiceHandle;
	}

	void AudioEngine::FreeVoice(VoiceHandle handle)
	{
		if (handle == InvalidVoiceHandle)
		{
			return;
		}

		VoiceContext* voiceCtx = impl->GetVoiceContext(handle);
		if (voiceCtx != nullptr)
		{
			FAudioVoice_DestroyVoice(voiceCtx->SourceVoice);

			voiceCtx->Source = InvalidSourceHandle;
			voiceCtx->VoiceFlags = VoiceState_Free;

			LogInfo(LogName, "Voice %lld has been freed", handle);
		}
	}

	SampleProvider* AudioEngine::GetSource(SourceHandle handle)
	{
		return impl->GetSource(handle);
	}
#pragma endregion

#pragma region Voice Implementation
	bool Voice::GetIsPlaying() const
	{
		return GetInternalFlag(VoiceState_Playing);
	}

	void Voice::SetIsPlaying(bool playing)
	{
		if (GetIsPlaying())
		{
			return;
		}

		SetInternalFlag(VoiceState_Playing, playing);

		VoiceContext* voiceCtx = GlobalInstance->impl->GetVoiceContext(Handle);
		if (voiceCtx != nullptr)
		{
			if (playing)
			{
				FAudioSourceVoice_Start(voiceCtx->SourceVoice, 0, FAUDIO_COMMIT_NOW);
				// NOTE: Make sure that the voice has something to play and that callback functions are actually called
				// Yes, I didn't think this through
				VoiceCallbacks::OnBufferEnd(voiceCtx);
			}
			else
			{
				FAudioSourceVoice_Stop(voiceCtx->SourceVoice, 0, FAUDIO_COMMIT_NOW);
				FAudioSourceVoice_FlushSourceBuffers(voiceCtx->SourceVoice);
			}
		}
	}

	size_t Voice::GetSamplePosition() const
	{
		VoiceContext* voiceCtx = GlobalInstance->impl->GetVoiceContext(Handle);
		if (voiceCtx != nullptr)
		{
			return voiceCtx->SamplePosition;
		}
	}

	void Voice::SetSamplePosition(size_t position)
	{
		VoiceContext* voiceCtx = GlobalInstance->impl->GetVoiceContext(Handle);
		if (voiceCtx != nullptr)
		{
			voiceCtx->SamplePosition = position;
		}
	}

	VoiceHandle Voice::GetHandle() const
	{
		return Handle;
	}

	bool Voice::GetInternalFlag(u16 flag) const
	{
		VoiceContext* ctx = GlobalInstance->impl->GetVoiceContext(Handle);

		if (ctx != nullptr)
		{
			return (ctx->VoiceFlags & flag) != 0;
		}

		return false;
	}

	void Voice::SetInternalFlag(u16 flag, bool value)
	{
		VoiceContext* ctx = GlobalInstance->impl->GetVoiceContext(Handle);

		if (ctx != nullptr)
		{
			if (value)
			{
				ctx->VoiceFlags |= flag;
			}
			else
			{
				ctx->VoiceFlags &= ~flag;
			}
		}
	}
}
#pragma endregion
