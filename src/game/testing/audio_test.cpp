#include <fmt/format.h>
#include <algorithm>
#include "audio_test.h"
#include "util/logging.h"
#include "io/binary_reader.h"
#include "global_res.h"
#include <fstream>
#include <FAudio.h>
#if defined(_WIN32)
#include <Windows.h>
#endif

namespace Testing
{
	using namespace Common;
	using namespace GFX;
	using namespace Logging;
	using namespace Input;
	using GFX::LowLevel::ClearFlags;
	using GFXBackend = GFX::LowLevel::Backend;

	using std::string;
	using std::fstream;
	using std::ios;

	constexpr const char* LogName{ "Testing::Audio" };
	constexpr const char* LogName_FAudio{ "FAudio" };

	constexpr u32 DefaultOutputChannels{ 2 };
	constexpr u32 DefaultSampleRate{ 44100 };
	constexpr u32 DefaultBitsPerSample{ 16 };

	constexpr u32 BufferedSamplesAmount{ 2048 };
	constexpr u32 BufferedSamplesBytes{ BufferedSamplesAmount * sizeof(i16) };

	// NOTE: Return value is amount of bytes
	size_t MixChannels(const i16* sourceSamples, size_t firstSampleIndex, size_t sampleAmount, i16* outputSamples, int channelCount)
	{
		if (channelCount < 2)
		{
			// Duplicate one sample into multiple channels
			const i16* copySource = &sourceSamples[firstSampleIndex / 2];
			for (size_t i = 0; i < sampleAmount / 2; i++)
			{
				outputSamples[i * 2 + 0] = copySource[i];
				outputSamples[i * 2 + 1] = copySource[i];
			}
			return sampleAmount;
		}
		else
		{
			// Copy as-is
			const i16* copySource = &sourceSamples[firstSampleIndex];
			size_t copyAmount = sampleAmount * sizeof(i16);

			SDL_memcpy(outputSamples, copySource, copyAmount);
			return sampleAmount;
		}
	}

	struct VoiceCtx
	{
		FAudioVoice* sourceVoice{};
		FAudioWaveFormatEx sourceFormat{};

		i16* sourceSamples{ nullptr };
		size_t sourceSampleCount = 0;

		// NOTE: This is the amount of samples, not the amount of bytes
		size_t sourceSamplesRead = 0;

		i16* processedData{ nullptr };
		size_t processedDataCapacity = 0;

		bool stopDecoding = false;

		void ResetPlaybackState()
		{
			sourceSamplesRead = 0;
			stopDecoding = false;
		}

		void DecodeAndSumbitNextBuffer()
		{
			if (sourceSamplesRead >= sourceSampleCount || stopDecoding)
			{
				return;
			}

			FAudioBuffer buffer = {};

			size_t firstSampleIndex = sourceSamplesRead;
			size_t sampleAmountToMix = std::min<size_t>(processedDataCapacity, sourceSampleCount - sourceSamplesRead);
			sourceSamplesRead += sampleAmountToMix;

			size_t sampleAmountToSubmit = MixChannels(sourceSamples, firstSampleIndex, sampleAmountToMix, processedData, sourceFormat.nChannels);

			buffer.pContext = this;
			buffer.pAudioData = (u8*)processedData;
			buffer.AudioBytes = sampleAmountToSubmit * sizeof(i16);

			if (sourceSamplesRead >= sourceSampleCount)
			{
				buffer.Flags = FAUDIO_END_OF_STREAM;
			}
			
			FAudioSourceVoice_SubmitSourceBuffer(sourceVoice, &buffer, NULL);
		}
	};

	void OnBufferEnd(FAudioVoiceCallback* callback, void* pBufferContext)
	{
		VoiceCtx* ctx = static_cast<VoiceCtx*>(pBufferContext);
		ctx->DecodeAndSumbitNextBuffer();
	}

	struct AudioTest::StateInternal
	{
		Game* game = nullptr;
		GFXBackend* gfxBackend = nullptr;
		GFX::SpriteRenderer* spriteRenderer = nullptr;
		GFX::Font* debugFont = nullptr;

		// NOTE: Keybinds
		Keyboard* keyboard = Keyboard::GetInstance();
		KeyBind PlaySoundKeybind = KeyBind(keyboard, SDL_SCANCODE_SPACE, KeyBind::UnsetScancode);

		// NOTE: FAudio members
		FAudio* faudio{ nullptr };
		FAudioMasteringVoice* masteringVoice{ nullptr };

		FAudioVoice* sourceVoice{ nullptr };
		VoiceCtx internalCtx{};
		FAudioVoiceCallback sourceVoiceCallbacks{};

		FAudioWaveFormatEx voiceFormat{};
		FAudioVoiceState sourceVoiceState{};

		// NOTE: Debug state
		fmt::memory_buffer debugText = fmt::memory_buffer();

		// NOTE: State functions
		StateInternal(Game* game) : game(game)
		{
			gfxBackend = game->GetGraphicsBackend();
		}

		bool Initialize()
		{
			u32 result = 0;

			if ((result = FAudioCreate(&faudio, 0, FAUDIO_DEFAULT_PROCESSOR)) != 0)
			{
				LogError(LogName_FAudio, "Failed to create FAudio engine. Error: %04x", result);
				return false;
			}

			LogInfo(LogName_FAudio, "Linked FAudio version: %u", FAudioLinkedVersion());

			if ((result = FAudio_CreateMasteringVoice(faudio, &masteringVoice, DefaultOutputChannels, DefaultSampleRate, 0, 0, NULL)) != 0)
			{
				LogError(LogName_FAudio, "Failed to create FAudio mastering voice. Error: %04x", result);
				FAudio_Release(faudio);
				return false;
			}

			voiceFormat.wFormatTag = FAUDIO_FORMAT_PCM;
			voiceFormat.nChannels = DefaultOutputChannels;
			voiceFormat.nSamplesPerSec = DefaultSampleRate;
			voiceFormat.wBitsPerSample = DefaultBitsPerSample;
			voiceFormat.nBlockAlign = (DefaultOutputChannels * DefaultBitsPerSample) / 8;
			voiceFormat.nAvgBytesPerSec = DefaultSampleRate * DefaultOutputChannels * (DefaultBitsPerSample / 8);

			internalCtx.processedData = new i16[BufferedSamplesAmount];
			internalCtx.processedDataCapacity = BufferedSamplesAmount;
			sourceVoiceCallbacks.OnBufferEnd = (OnBufferEndFunc)OnBufferEnd;

			FAudio_CreateSourceVoice(faudio, &sourceVoice, &voiceFormat, 0, FAUDIO_DEFAULT_FREQ_RATIO, &sourceVoiceCallbacks, NULL, NULL);
			internalCtx.sourceVoice = sourceVoice;

			spriteRenderer = GlobalResources::SpriteRenderer;
			debugFont = GlobalResources::DebugFont;
			return true;
		}

		bool LoadContent()
		{
			size_t fileSize = 0;

#if defined(_WIN32)
			WIN32_FILE_ATTRIBUTE_DATA fileAttribData{};
			GetFileAttributesExA("diva/sounds/test_wav.wav", GetFileExInfoStandard, &fileAttribData);

			fileSize = (static_cast<size_t>(fileAttribData.nFileSizeHigh) << 32) | static_cast<size_t>(fileAttribData.nFileSizeLow);
#endif

			fstream wavFile = fstream("diva/sounds/test_wav.wav", ios::in | ios::binary);
			BinaryReader reader = BinaryReader(&wavFile, 0, fileSize);
			size_t wavFileSize = reader.GetSize();

			// WAVRIFFHEADER
			char chunkID[4]{};
			reader.Read(chunkID, 4);
			if (SDL_memcmp(chunkID, "RIFF", 4) != 0)
			{
				wavFile.close();
				return false;
			}

			reader.ReadU32(); // File size
			u32 chunkSize = 0;

			reader.Read(chunkID, 4);
			if (SDL_memcmp(chunkID, "WAVE", 4) != 0)
			{
				wavFile.close();
				return false;
			}

			// FORMATCHUNK
			SeekWavChunk(reader, "fmt ", &chunkSize);
			reader.Read(&internalCtx.sourceFormat, chunkSize);

			SeekWavChunk(reader, "data", &chunkSize);
			size_t samplesAmount = chunkSize / sizeof(i16);

			internalCtx.sourceSamples = new i16[samplesAmount];
			reader.Read(internalCtx.sourceSamples, chunkSize);

			internalCtx.sourceSampleCount = samplesAmount;

			wavFile.close();
			return true;
		}

		void Destroy()
		{
			delete[] internalCtx.processedData;
			delete[] internalCtx.sourceSamples;
			debugText.clear();

			FAudioVoice_DestroyVoice(sourceVoice);
			FAudioVoice_DestroyVoice(masteringVoice);
			FAudio_Release(faudio);
			LogInfo(LogName_FAudio, "FAudio mastering voice and engine destroyed");
		}

		void Update()
		{
			if (PlaySoundKeybind.IsTapped(nullptr, nullptr))
			{
				FAudioSourceVoice_GetState(sourceVoice, &sourceVoiceState, 0);

				internalCtx.ResetPlaybackState();
				internalCtx.DecodeAndSumbitNextBuffer();
				FAudioSourceVoice_Start(sourceVoice, 0, FAUDIO_COMMIT_NOW);
			}

			debugText.clear();
			fmt::format_to(std::back_inserter(debugText), "Sample amount: {}\n", internalCtx.sourceSampleCount);
			fmt::format_to(std::back_inserter(debugText), "Samples read: {}", internalCtx.sourceSamplesRead);
		}

		void Draw()
		{
			constexpr Color ClearColor{ 24, 24, 24, 255 };
			gfxBackend->Clear(ClearFlags::GFX_CLEAR_COLOR, ClearColor, 1.0f, 0);
			gfxBackend->SetBlendState(&GFX::LowLevel::DefaultBlendStates::AlphaBlend);

			debugFont->PushString(spriteRenderer, "Press Space to play a sound effect", vec2(0.0f), vec2(1.0f), DefaultColors::White);
			debugFont->PushString(spriteRenderer, debugText.begin(), debugText.size(), vec2(0.0f, 24.0f), vec2(1.0f), DefaultColors::White);
			spriteRenderer->RenderSprites(nullptr);

			gfxBackend->SwapBuffers();
		}

		size_t SeekWavChunk(BinaryReader& reader, const char id[4], u32* chunkSize)
		{
			char chunkName[4]{};
			u32 internalChunkSize = 0;

			do
			{
				reader.Seek(SeekOrigin::Current, static_cast<size_t>(internalChunkSize));
				reader.Read(chunkName, 4);

				if (SDL_memcmp(chunkName, "INFO", 4) == 0)
				{
					reader.Read(chunkName, 4);
				}

				internalChunkSize = reader.ReadU32();
			} while (SDL_memcmp(chunkName, id, 4) != 0);

			*chunkSize = internalChunkSize;
			return reader.GetLocation();
		}
	};

	AudioTest::AudioTest()
	{
	}

	AudioTest::~AudioTest()
	{
	}

	bool AudioTest::Initialize()
	{
		stateInternal = new StateInternal(game);
		return stateInternal->Initialize();
	}

	bool AudioTest::LoadContent()
	{
		return stateInternal->LoadContent();
	}

	void AudioTest::UnloadContent()
	{
	}

	void AudioTest::Destroy()
	{
		stateInternal->Destroy();
		delete stateInternal;
	}

	void AudioTest::OnResize(u32 newWidth, u32 newHeight)
	{
	}

	void AudioTest::Update()
	{
		stateInternal->Update();
	}

	void AudioTest::Draw()
	{
		stateInternal->Draw();
	}
}
