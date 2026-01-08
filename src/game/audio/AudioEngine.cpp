#include <array>
#include <vector>
#include "AudioEngine.h"
#include "Decoding/DecoderFactory.h"
#include "io/File.h"
#include "util/logging.h"

namespace Starshine::Audio
{
	using namespace Logging;

	constexpr const char* LogName = "Starshine::Audio::AudioEngine";

	AudioEngine* Instance = nullptr;
	void AudioEngine_SDLCallback(void* userdata, Uint8* stream, int size);

	struct SourceData
	{
		ISampleProvider* SampleProvider{};

		size_t LoopStart{};
		size_t LoopEnd{};
	};

	struct VoiceContext
	{
		SourceHandle Source{};

		bool Allocated{};
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
				Logging::LogError(LogName, "Failed to open SDL Audio device. Error: %s", SDL_GetError());
				return false;
			}

			sdlDevID = result;

			Logging::LogInfo(LogName, 
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
			SDL_CloseAudioDevice(sdlDevID);
		}

		void QueueAudio(f32* stream, size_t length)
		{
			SDL_memset(&mixingBuffer[0], 0, length * sizeof(f32));

			for (auto& it = voiceContexts.begin(); it != voiceContexts.end(); it++)
			{
				if (it->Source != SourceHandle::Invalid && it->Allocated && it->Playing)
				{
					const SourceData* source = GetSourceData(it->Source);
					ISampleProvider* sampleProvider = source->SampleProvider;
					size_t channels = sampleProvider->GetChannelCount();
					size_t endPosition = it->Looped ? (source->LoopEnd) : (sampleProvider->GetSampleAmount() / channels);
				
					if (it->FramePosition >= endPosition)
					{
						if (!it->Looped)
						{
							it->Playing = false;
							continue;
						}

						it->FramePosition = source->LoopStart;
					}

					size_t samplesToRead = length / (DefaultChannelCount / channels);
					size_t readSamples = sampleProvider->ReadSamples(&workingBuffer[0], it->FramePosition * channels, length / (DefaultChannelCount / channels));

					if (it->Looped && readSamples < samplesToRead)
					{
						size_t bufferRemainder = samplesToRead - readSamples;
						readSamples = sampleProvider->ReadSamples(
							&workingBuffer[bufferRemainder], 
							source->LoopStart * channels, 
							bufferRemainder / (DefaultChannelCount / channels));

						it->FramePosition = source->LoopStart + readSamples;
						readSamples = samplesToRead;
					}

					for (size_t pos = 0; pos < readSamples; pos += channels)
					{
						if (channels == 1)
						{
							f32 sample = ConvertSampleFromI16ToF32(workingBuffer[pos]);
							mixingBuffer[pos * 2 + 0] += sample;
							mixingBuffer[pos * 2 + 1] += sample;

							mixingBuffer[pos * 2 + 0] = SDL_clamp(mixingBuffer[pos * 2 + 0], -1.0f, 1.0f);
							mixingBuffer[pos * 2 + 1] = SDL_clamp(mixingBuffer[pos * 2 + 1], -1.0f, 1.0f);

							it->FramePosition++;
						}
						else // 2 channels
						{
							mixingBuffer[pos + 0] += ConvertSampleFromI16ToF32(workingBuffer[pos + 0]);
							mixingBuffer[pos + 1] += ConvertSampleFromI16ToF32(workingBuffer[pos + 1]);

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

		void UnloadSource(SourceHandle handle)
		{
			if (handle != SourceHandle::Invalid && static_cast<size_t>(handle) < registeredSources.size())
			{
				SourceData& source = registeredSources.at(static_cast<size_t>(handle));
				if (source.SampleProvider != nullptr)
				{
					source.SampleProvider->Destroy();
					delete source.SampleProvider;

					LogInfo(LogName, "Audio source with handle %u has been unloaded", handle);

					for (size_t i = 0; i < voiceContexts.size(); i++)
					{
						VoiceContext* voiceCtx = &voiceContexts[i];
						if (voiceCtx->Source == handle)
						{
							voiceCtx->Source = SourceHandle::Invalid;
							LogInfo(LogName, "The audio source of the voice with handle %llu has been invalidated", i);
							break;
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
		u8* fileData = nullptr;
		size_t fileSize = IO::File::ReadAllBytes(filePath, &fileData);

		if (fileData != nullptr && fileSize > 0)
		{
			SourceHandle handle = AudioEngine::GetInstance()->LoadSource(fileData, fileSize);
			delete[] fileData;

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
		}
	}
}
