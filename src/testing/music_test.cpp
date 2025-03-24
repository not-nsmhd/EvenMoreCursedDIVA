#include "music_test.h"
#include "global_res.h"
#include "util/logging.h"
#include <fstream>

using std::ios;
using std::fstream;

namespace Testing
{
	static GFX::Font* debugFont;

	MusicTest* MusicTest::instance = new MusicTest();

	MusicTest* MusicTest::GetInstance()
	{
		return instance;
	}
	
	bool MusicTest::Initialize()
	{
		if (FAudioCreate(&faudio, 0, FAUDIO_DEFAULT_PROCESSOR) != 0)
		{
			return false;
		}

		if (FAudio_CreateMasteringVoice(faudio, &masteringVoice, FAUDIO_DEFAULT_CHANNELS, FAUDIO_DEFAULT_SAMPLERATE, 0, 0, NULL) != 0)
		{
			return false;
		}

		debugFont = GlobalResources::DebugFont;
		return true;
	}
	
	bool MusicTest::LoadContent()
	{
		fstream vorbisFileStream = fstream(fileSystem->GetContentFilePath("music/test.ogg"), ios::in | ios::binary);

		vorbisFileStream.seekg(0, ios::end);
		vorbisFileSize = vorbisFileStream.tellg();
		vorbisFileOffset = 0;
		vorbisFileStream.seekg(0, ios::beg);

		vorbisFileData = new u8[vorbisFileSize];
		vorbisFileStream.read((char*)vorbisFileData, vorbisFileSize);
		vorbisFileStream.close();

		vorbisCallbacks = 
		{
			(size_t (*)(void *, size_t, size_t, void *))&callbackRead,
			(int (*)(void *, ogg_int64_t, int))&callbackSeek,
			(int (*)(void *))&callbackClose,
			(long (*)(void *))&callbackTell
		};

		if (ov_open_callbacks(vorbisFileData, &ovFile, NULL, 0, vorbisCallbacks) < 0)
		{
			Logging::LogError("MusicTest", "Failed to open Ogg Vorbis file with custom callbacks");
			delete[] vorbisFileData;
			return false;
		}

		{
			vorbis_info* oggInfo = ov_info(&ovFile, -1);
			vorbis_comment* oggComments = ov_comment(&ovFile, -1);

			readLoopValues(oggComments);

			FAudioWaveFormatEx wfx = {};
			wfx.wFormatTag = FAUDIO_FORMAT_PCM;
			wfx.nChannels = oggInfo->channels;
			wfx.nSamplesPerSec = oggInfo->rate;
			wfx.wBitsPerSample = 16;
			wfx.nBlockAlign = (oggInfo->channels * wfx.wBitsPerSample) / 8;
			wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
			wfx.cbSize = 0;

			streamingCallbacks.OnBufferEnd = (OnBufferEndFunc)&MusicTest::decodeNextBuffer;

			if (FAudio_CreateSourceVoice(faudio, &streamingVoice, &wfx, 0, 2.0f, &streamingCallbacks, NULL, NULL) != 0)
			{
				return false;
			}
		}

		spriteRenderer.Initialize(graphicsBackend);
		return true;
	}
	
	void MusicTest::UnloadContent()
	{
		if (playing)
		{
			FAudioSourceVoice_Stop(streamingVoice, 0, FAUDIO_COMMIT_NOW);
		}

		spriteRenderer.Destroy();

		ov_clear(&ovFile);

		delete[] vorbisFileData;
		vorbisFileSize = 0;
		vorbisFileOffset = 0;
	}
	
	void MusicTest::Destroy()
	{
		FAudioVoice_DestroyVoice(streamingVoice);
		FAudioVoice_DestroyVoice(masteringVoice);
		FAudio_Release(faudio);
	}
	
	void MusicTest::OnResize(u32 newWidth, u32 newHeight)
	{
	}
	
	void MusicTest::Update()
	{
		if (keyboardState->IsKeyTapped(SDL_SCANCODE_SPACE) && playing == false)
		{
			decodeNextBuffer(&streamingCallbacks, NULL);
			FAudioSourceVoice_Start(streamingVoice, 0, FAUDIO_COMMIT_NOW);
			playing = true;
		}

		SDL_memset(debugStateString, 0, 1024);

		int offset = 0;
		offset += SDL_snprintf(debugStateString, 1023, "\nStream location (samples): %d\n", streamLocation_samples);
		offset += SDL_snprintf(debugStateString + offset, 1023, "Stream location (seconds): %.3f\n", (float)streamLocation_samples / (float)44100);

		if (loopEnd_samples > 0)
		{
			offset += SDL_snprintf(debugStateString + offset, 1023, "Stream loop (seconds): %.3f - %.3f\n", 
				(float)loopStart_samples / (float)44100, (float)loopEnd_samples/ (float)44100);
		}
	}
	
	void MusicTest::Draw()
	{
		graphicsBackend->Clear(GFX::LowLevel::ClearFlags::GFX_CLEAR_COLOR, Common::Color(0, 24, 24, 255), 1.0f, 0);

		debugFont->PushString(spriteRenderer, "Press 'Space' key to start streaming", glm::vec2(0.0f), glm::vec2(1.0f), Common::DefaultColors::White);

		if (playing)
		{
			debugFont->PushString(spriteRenderer, debugStateString, 1023, glm::vec2(0.0f), glm::vec2(1.0f), Common::DefaultColors::White);
		}

		spriteRenderer.RenderSprites(nullptr);

		graphicsBackend->SwapBuffers();
	}
	
	void MusicTest::readLoopValues(vorbis_comment* metadata)
	{
		for (int i = 0; i < metadata->comments; i++)
		{
			char* metadataKeyValue = metadata->user_comments[i];
			int metadataLength = metadata->comment_lengths[i];

			size_t keyNameSize = (size_t)(SDL_strstr(metadataKeyValue, "=") - metadataKeyValue);
			char* keyValueStart = metadataKeyValue + keyNameSize + 1;
			size_t keyValueSize = metadataLength - (size_t)(keyValueStart);

			if (SDL_strncmp("LoopStart", metadataKeyValue, keyNameSize) == 0)
			{
				loopStart_samples = SDL_atoi(keyValueStart);
			}

			if (SDL_strncmp("LoopEnd", metadataKeyValue, keyNameSize) == 0)
			{
				loopEnd_samples = SDL_atoi(keyValueStart);
			}
		}
	}
	
	size_t MusicTest::callbackRead(void* ptr, size_t size, size_t nmemb, void* dataSource)
	{
		if (dataSource != instance->vorbisFileData)
		{
			return 0;
		}

		size_t copySize = SDL_min(size * nmemb, instance->vorbisFileSize - instance->vorbisFileOffset);

		SDL_memcpy(ptr, instance->vorbisFileData + instance->vorbisFileOffset, copySize);
		instance->vorbisFileOffset += copySize;

		return copySize;
	}
	
	int MusicTest::callbackSeek(void* ptr, ogg_int64_t offset, int whence)
	{
		switch (whence)
		{
			case SEEK_SET:
				instance->vorbisFileOffset = offset;
				break;
			case SEEK_CUR:
				instance->vorbisFileOffset += offset;
				break;
			case SEEK_END:
				instance->vorbisFileOffset = instance->vorbisFileSize - offset - 1;
				break;
		}

		if (instance->vorbisFileOffset >= instance->vorbisFileSize)
		{
			instance->vorbisFileOffset = 0;
			return OV_EINVAL;
		}

		return 0;
	}
	
	int MusicTest::callbackClose(void* dataSource)
	{
		return 0;
	}
	
	long MusicTest::callbackTell(void* dataSource)
	{
		return instance->vorbisFileOffset;
	}
	
	void MusicTest::decodeNextBuffer(FAudioVoiceCallback* callback, void* context)
	{
		if (!instance->eofReached)
		{
			if (instance->loopEnd_samples > 0 && instance->streamLocation_samples >= instance->loopEnd_samples)
			{
				ov_pcm_seek(&instance->ovFile, instance->loopStart_samples);
				instance->streamLocation_samples = instance->loopStart_samples;
			}

			long result = ov_read(&instance->ovFile, (char*)instance->pcmData, PCM_DATA_SIZE, 0, 2, 1, &instance->currentVorbisBitstream);

			if (result == 0 && (instance->loopStart_samples == 0 || instance->loopEnd_samples == 0))
			{
				instance->eofReached = true;
			}
			else if (result < 0)
			{
				// uiuiuiuiuiu something
			}
			else
			{
				instance->streamingBuffer.AudioBytes = result;
				instance->streamingBuffer.pAudioData = instance->pcmData;
				instance->streamingBuffer.Flags = (instance->eofReached ? FAUDIO_END_OF_STREAM : 0);
				FAudioSourceVoice_SubmitSourceBuffer(instance->streamingVoice, &instance->streamingBuffer, NULL);

				instance->streamLocation_samples += result / (sizeof(u16) * 2);
			}
		}
	}
}
