#include <array>
#include <vector>
#include "AudioEngine.h"
#include "Decoding/DecoderFactory.h"
#include "SampleProvider/StreamingSampleProvider.h"
#include "IO/Path/File.h"
#include "Common/Logging/Logging.h"

namespace Starshine::Audio
{
	constexpr const char* LogName = "Starshine::Audio::AudioEngine";

	AudioEngine* Instance = nullptr;
	void AudioEngine_SDLCallback(void* userdata, Uint8* stream, int size);

	struct SourceData
	{
		ISampleProvider* SampleProvider{};

		size_t LoopStart{};
		size_t LoopEnd{};

		// NOTE: Only one voice at a time can play audio from a streaming source
		VoiceHandle BoundVoice{ VoiceHandle::Invalid };
	};

	struct VoiceContext
	{
		SourceHandle Source{ SourceHandle::Invalid };
		f32 Volume{ 1.0f };

		bool Allocated{};
		bool DeallocateOnEnd{};

		bool Playing{};
		bool Looped{};

		size_t FramePosition{};
	};

	constexpr f32 ConvertSampleFromI16ToF32(i16 s) { return static_cast<f32>(s) / static_cast<f32>(std::numeric_limits<i16>::max()); };
	constexpr i16 ConvertSampleFromF32ToI16(f32 s) { return static_cast<i16>(s * static_cast<f32>(std::numeric_limits<i16>::max())); };

	struct AudioEngine::Impl
	{
		SDL_AudioSpec sdlSpec = {};
		SDL_AudioDeviceID sdlDevID = 0;

		std::array<VoiceContext, MaxSimultaneousVoices> voiceContexts;
		std::vector<SourceData> registeredSources;

		std::array<i16, DefaultSampleBufferSize> workingBuffer;
		std::array<f32, DefaultSampleBufferSize> mixingBuffer;

		bool Initialize()
		{
			SDL_AudioSpec desiredSpec = {};
			desiredSpec.channels = DefaultChannelCount;
			desiredSpec.freq = DefaultSampleRate;
			desiredSpec.format = AUDIO_F32SYS;
			desiredSpec.samples = DefaultSampleBufferSize / DefaultChannelCount;
			desiredSpec.callback = AudioEngine_SDLCallback;
			desiredSpec.userdata = NULL;

			int result = 0;
			if ((result = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &sdlSpec, 0)) == 0)
			{
				LogError(LogName, "Failed to open SDL Audio device. Error: %s", SDL_GetError());
				return false;
			}

			sdlDevID = result;

			LogInfo(LogName, 
				"SDL Audio Device (ID %u, Driver: %s) spec:\n"
				"\tsdlSpec.channels: %u\n" 
				"\tsdlSpec.freq: %d\n"
				"\tsdlSpec.samples: %u\n" 
				"\tsdlSpec.format: 0x%x\n", 
				sdlDevID, SDL_GetCurrentAudioDriver(), sdlSpec.channels, sdlSpec.freq, sdlSpec.samples, sdlSpec.format);

			constexpr size_t initialSourceCapacity = 64;
			registeredSources.reserve(initialSourceCapacity);

			SDL_PauseAudioDevice(sdlDevID, 0);

			return true;
		}

		void Destroy()
		{
			SDL_PauseAudioDevice(sdlDevID, 1);

			for (auto it = registeredSources.begin(); it != registeredSources.end(); it++)
			{
				if (it->SampleProvider == nullptr)
				{
					continue;
				}

				it->SampleProvider->Destroy();
				delete it->SampleProvider;
				it->SampleProvider = nullptr;
			}

			for (auto it = voiceContexts.begin(); it != voiceContexts.end(); it++)
			{
				if (!it->Allocated)
				{
					continue;
				}

				it->Playing = false;
				it->Allocated = false;
				it->Source = SourceHandle::Invalid;
			}

			SDL_CloseAudioDevice(sdlDevID);
		}

		void QueueAudio(f32* stream, size_t length)
		{
			SDL_memset(&mixingBuffer[0], 0, length * sizeof(f32));

			int currentVoice = -1;
			for (auto it = voiceContexts.begin(); it != voiceContexts.end(); it++)
			{
				currentVoice++;
				if (it->Source != SourceHandle::Invalid && it->Allocated && it->Playing)
				{
					const SourceData* source = GetSourceData(it->Source);

					if (source == nullptr)
					{
						continue;
					}

					ISampleProvider* sampleProvider = source->SampleProvider;
					bool streaming = sampleProvider->IsStreamingOnly();

					if (streaming && (static_cast<int>(source->BoundVoice) != currentVoice))
					{
						continue;
					}

					size_t channels = sampleProvider->GetChannelCount();
					size_t endPosition = sampleProvider->GetSampleAmount() / channels;
				
					if (it->FramePosition >= endPosition)
					{
						if (!it->Looped)
						{
							it->Playing = false;

							if (it->DeallocateOnEnd)
							{
								it->Allocated = false;
								it->DeallocateOnEnd = false;
								it->Source = SourceHandle::Invalid;
								it->FramePosition = 0;
								it->Volume = 1.0f;
							}

							continue;
						}

						it->FramePosition = source->LoopStart;
					}

					size_t samplesToRead = length / (DefaultChannelCount / channels);
					size_t readSamples = 0;

					if (streaming)
					{
						readSamples = sampleProvider->GetNextSamples(&workingBuffer[0], samplesToRead);
					}
					else
					{
						readSamples = sampleProvider->ReadSamples(&workingBuffer[0], it->FramePosition * channels, samplesToRead);
					}

					if (it->Looped && readSamples < samplesToRead)
					{
						size_t bufferRemainder = samplesToRead - readSamples;
						if (streaming)
						{
							sampleProvider->Seek(source->LoopStart * channels);
							readSamples = sampleProvider->GetNextSamples(
								&workingBuffer[bufferRemainder],
								bufferRemainder / (DefaultChannelCount / channels));
						}
						else
						{
							readSamples = sampleProvider->ReadSamples(
								&workingBuffer[bufferRemainder],
								source->LoopStart * channels,
								bufferRemainder / (DefaultChannelCount / channels));
						}

						it->FramePosition = source->LoopStart + readSamples;
						readSamples = samplesToRead;
					}

					for (size_t pos = 0; pos < readSamples; pos += channels)
					{
						if (channels == 1)
						{
							f32 sample = ConvertSampleFromI16ToF32(workingBuffer[pos]) * it->Volume;
							mixingBuffer[pos * 2 + 0] += sample;
							mixingBuffer[pos * 2 + 1] += sample;

							mixingBuffer[pos * 2 + 0] = SDL_clamp(mixingBuffer[pos * 2 + 0], -1.0f, 1.0f);
							mixingBuffer[pos * 2 + 1] = SDL_clamp(mixingBuffer[pos * 2 + 1], -1.0f, 1.0f);

							it->FramePosition++;
						}
						else // 2 channels
						{
							mixingBuffer[pos + 0] += ConvertSampleFromI16ToF32(workingBuffer[pos + 0]) * it->Volume;
							mixingBuffer[pos + 1] += ConvertSampleFromI16ToF32(workingBuffer[pos + 1]) * it->Volume;

							mixingBuffer[pos + 0] = SDL_clamp(mixingBuffer[pos + 0], -1.0f, 1.0f);
							mixingBuffer[pos + 1] = SDL_clamp(mixingBuffer[pos + 1], -1.0f, 1.0f);

							it->FramePosition++;
						}
					}
				}
			}

			SDL_LockAudioDevice(sdlDevID);

			SDL_memcpy(stream, &mixingBuffer[0], length * sizeof(f32));

			SDL_UnlockAudioDevice(sdlDevID);
		}

		VoiceContext* GetVoiceContext(VoiceHandle handle)
		{
			if (handle != VoiceHandle::Invalid && static_cast<size_t>(handle) < voiceContexts.size())
			{
				VoiceContext* ctx = &voiceContexts[static_cast<size_t>(handle)];
				if (ctx->Allocated)
				{
					return ctx;
				}
			}
			return nullptr;
		}

		SourceData* GetSourceData(SourceHandle handle)
		{
			if (handle != SourceHandle::Invalid && static_cast<size_t>(handle) < registeredSources.size())
			{
				SourceData* data = &registeredSources[static_cast<size_t>(handle)];
				if (data->SampleProvider != nullptr)
				{
					return data;
				}
			}
			return nullptr;
		}

		VoiceHandle AllocateVoice(SourceHandle source)
		{
			if (source == SourceHandle::Invalid)
			{
				return VoiceHandle::Invalid;
			}

			SourceData* sourceData = GetSourceData(source);

			for (size_t i = 0; i < MaxSimultaneousVoices; i++)
			{
				VoiceContext& voiceCtx = voiceContexts.at(i);

				if (voiceCtx.Allocated)
				{
					continue;
				}

				voiceCtx.Allocated = true;
				voiceCtx.Source = source;
				voiceCtx.FramePosition = 0;

				if (sourceData->SampleProvider->IsStreamingOnly())
				{
					sourceData->BoundVoice = static_cast<VoiceHandle>(i);
				}

				return static_cast<VoiceHandle>(i);
			}

			return VoiceHandle::Invalid;
		}

		void ReleaseVoice(VoiceHandle handle)
		{
			if (handle != VoiceHandle::Invalid)
			{
				VoiceContext* voiceCtx = GetVoiceContext(handle);
				if (voiceCtx != nullptr)
				{
					voiceCtx->Allocated = false;
					voiceCtx->Source = SourceHandle::Invalid;

					voiceCtx->FramePosition = 0;
					voiceCtx->Playing = false;
					voiceCtx->Looped = false;
				}
			}
		}

		SourceHandle RegisterSource(ISampleProvider* sampleProvider)
		{
			if (sampleProvider == nullptr)
			{
				return SourceHandle::Invalid;
			}

			for (size_t i = 0; i < registeredSources.size(); i++)
			{
				SourceData& source = registeredSources.at(i);

				if (source.SampleProvider != nullptr)
				{
					continue;
				}

				source.SampleProvider = sampleProvider;
				LogInfo(LogName, "Audio source with %llu samples has been registered (handle: %u, previously allocated)", sampleProvider->GetSampleAmount(), i);

				return static_cast<SourceHandle>(i);
			}

			registeredSources.push_back(SourceData{ sampleProvider, 0, 0 });
			LogInfo(LogName, "Audio source with %llu samples has been registered (handle: %u)", sampleProvider->GetSampleAmount(), registeredSources.size() - 1);
			return static_cast<SourceHandle>(registeredSources.size() - 1);
		}

		void BindStreamingSourceToANewVoice(SourceHandle source, VoiceHandle newVoice)
		{
			if (source != SourceHandle::Invalid && newVoice != VoiceHandle::Invalid)
			{
				SourceData* sourceData = GetSourceData(source);
				if (sourceData != nullptr)
				{
					if (sourceData->SampleProvider->IsStreamingOnly() && sourceData->BoundVoice != VoiceHandle::Invalid)
					{
						VoiceContext* oldVoice = GetVoiceContext(sourceData->BoundVoice);
						oldVoice->Playing = false;
						oldVoice->FramePosition = 0;
						oldVoice->Source = SourceHandle::Invalid;

						sourceData->BoundVoice = newVoice;
					}
				}
			}
		}

		SourceHandle LoadSource(const void* encodedData, size_t encodedDataSize)
		{
			if (encodedData != nullptr && encodedDataSize > 0)
			{
				ISampleProvider* sampleProvider = DecoderFactory::GetInstance()->DecodeFileData("", encodedData, encodedDataSize);
				if (sampleProvider == nullptr)
				{
					return SourceHandle::Invalid;
				}

				SourceHandle handle = RegisterSource(sampleProvider);

				SourceData* sourceData = GetSourceData(handle);
				sourceData->LoopStart = sampleProvider->GetLoopStart_Frames();
				sourceData->LoopEnd = sampleProvider->GetLoopEnd_Frames();

				return handle;
			}

			return SourceHandle::Invalid;
		}

		SourceHandle LoadStreamingSource(const void* encodedData, size_t encodedDataSize)
		{
			if (encodedData != nullptr && encodedDataSize > 0)
			{
				StreamingSampleProvider* sampleProvider = new StreamingSampleProvider(reinterpret_cast<const u8*>(encodedData), encodedDataSize);
				if (sampleProvider == nullptr)
				{
					return SourceHandle::Invalid;
				}

				SourceHandle handle = RegisterSource(sampleProvider);

				SourceData* sourceData = GetSourceData(handle);
				sourceData->LoopStart = sampleProvider->GetLoopStart_Frames();
				sourceData->LoopEnd = sampleProvider->GetLoopEnd_Frames();

				return handle;
			}

			return SourceHandle::Invalid;
		}

		void UnloadSource(SourceHandle handle)
		{
			if (handle != SourceHandle::Invalid && static_cast<size_t>(handle) < registeredSources.size())
			{
				SourceData& source = registeredSources.at(static_cast<size_t>(handle));
				if (source.SampleProvider != nullptr)
				{
					source.SampleProvider->Destroy();
					delete source.SampleProvider;
					source.SampleProvider = nullptr;

					LogInfo(LogName, "Audio source with handle %u has been unloaded", handle);

					for (size_t i = 0; i < voiceContexts.size(); i++)
					{
						VoiceContext* voiceCtx = &voiceContexts[i];
						if (voiceCtx->Source == handle)
						{
							voiceCtx->Source = SourceHandle::Invalid;
							if (!voiceCtx->DeallocateOnEnd)
							{
								LogInfo(LogName, "The audio source of the voice with handle %llu has been invalidated", i);
							}
						}
					}
				}
			}
		}
	};

	void AudioEngine_SDLCallback(void* userdata, Uint8* stream, int size)
	{
		AudioEngine::GetInstance()->QueueAudioCallback(reinterpret_cast<f32*>(stream), static_cast<size_t>(size / sizeof(f32)));
	}

	AudioEngine::AudioEngine() : impl(new Impl())
	{
	}

	AudioEngine::~AudioEngine()
	{
		delete impl;
	}

	void AudioEngine::CreateInstance()
	{
		if (Instance == nullptr)
		{
			Instance = new AudioEngine();
		}
	}

	void AudioEngine::DestroyInstance()
	{
		if (Instance != nullptr)
		{
			delete Instance;
			Instance = nullptr;
		}
	}

	AudioEngine* AudioEngine::GetInstance()
	{
		return Instance;
	}

	bool AudioEngine::Initialize()
	{
		return impl->Initialize();
	}

	void AudioEngine::Destroy()
	{
		impl->Destroy();
	}

	void AudioEngine::QueueAudioCallback(f32* stream, size_t length)
	{
		impl->QueueAudio(stream, length);
	}

	SourceHandle AudioEngine::RegisterSource(ISampleProvider* sampleProvider)
	{
		return impl->RegisterSource(sampleProvider);
	}

	SourceHandle AudioEngine::LoadSource(const void* encodedData, size_t encodedDataSize)
	{
		return impl->LoadSource(encodedData, encodedDataSize);
	}

	SourceHandle AudioEngine::LoadSource(std::string_view filePath)
	{
		std::unique_ptr<u8[]> fileData = nullptr;
		size_t fileSize = IO::File::ReadAllBytes(filePath, fileData);

		if (fileData != nullptr && fileSize > 0)
		{
			SourceHandle handle = LoadSource(fileData.get(), fileSize);
			fileData = nullptr;

			if (handle != SourceHandle::Invalid)
			{
				LogInfo(LogName, "Loaded file \"%s\"", filePath.data());
				return handle;
			}
			else
			{
				LogInfo(LogName, "Failed to load file \"%s\"", filePath.data());
				return SourceHandle::Invalid;
			}
		}

		return SourceHandle::Invalid;
	}

	SourceHandle AudioEngine::LoadStreamingSource(std::string_view filePath)
	{
		std::unique_ptr<u8[]> fileData = nullptr;
		size_t fileSize = IO::File::ReadAllBytes(filePath, fileData);

		if (fileData != nullptr && fileSize > 0)
		{
			SourceHandle handle = impl->LoadStreamingSource(fileData.get(), fileSize);

			if (handle != SourceHandle::Invalid)
			{
				LogInfo(LogName, "Loaded file \"%s\" for streaming", filePath.data());
				return handle;
			}
			else
			{
				LogInfo(LogName, "Failed to load file \"%s\" for streaming", filePath.data());
				return SourceHandle::Invalid;
			}
		}

		return SourceHandle::Invalid;
	}

	void AudioEngine::UnloadSource(SourceHandle handle)
	{
		impl->UnloadSource(handle);
	}

	VoiceHandle AudioEngine::AllocateVoice(SourceHandle source)
	{
		return impl->AllocateVoice(source);
	}

	void AudioEngine::FreeVoice(VoiceHandle handle)
	{
		impl->ReleaseVoice(handle);
	}

	void AudioEngine::PlaySound(SourceHandle source, f32 volume)
	{
		if (source == SourceHandle::Invalid)
		{
			return;
		}

		for (auto& voiceCtx : impl->voiceContexts)
		{
			if (voiceCtx.Allocated)
			{
				continue;
			}

			voiceCtx.Allocated = true;
			voiceCtx.DeallocateOnEnd = true;
			voiceCtx.Source = source;
			voiceCtx.Looped = false;
			voiceCtx.FramePosition = 0;
			voiceCtx.Volume = volume;
			voiceCtx.Playing = true;
			return;
		}
	}

	bool Voice::IsValid() const
	{
		auto& impl = Instance->impl;
		return (impl->GetVoiceContext(Handle) != nullptr);
	}

	bool Voice::IsPlaying() const
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			return ctx->Playing;
		}
	}

	void Voice::SetPlaying(bool play)
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			ctx->Playing = play;
		}
	}

	bool Voice::IsLooped() const
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			return ctx->Looped;
		}
	}

	void Voice::SetLoopState(bool loop)
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			ctx->Looped = loop;
		}
	}

	SourceHandle Voice::GetSource() const
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			return ctx->Source;
		}
	}

	void Voice::SetSource(SourceHandle handle)
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			ctx->Source = handle;
			impl->BindStreamingSourceToANewVoice(handle, this->Handle);
		}
	}

	size_t Voice::GetFramePosition() const
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			return ctx->FramePosition;
		}
	}

	void Voice::SetFramePosition(size_t position)
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			ctx->FramePosition = position;

			if (impl->GetSourceData(ctx->Source)->SampleProvider->IsStreamingOnly())
			{
				impl->GetSourceData(ctx->Source)->SampleProvider->Seek(position);
			}
		}
	}

	f32 Voice::GetVolume() const
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			return ctx->Volume;
		}
	}

	void Voice::SetVolume(f32 volume)
	{
		auto& impl = Instance->impl;
		if (auto ctx = impl->GetVoiceContext(Handle); ctx != nullptr)
		{
			ctx->Volume = volume;
		}
	}
}
