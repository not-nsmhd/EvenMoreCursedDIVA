#pragma once
#include <string>
#include "../game.h"
#include "../gfx/font.h"
#include "../gfx/sprite_renderer.h"
#include <vorbis/vorbisfile.h>
#include <FAudio.h>

namespace Testing
{
	class MusicTest : public GameState
	{
	public:
		MusicTest() {};

		static MusicTest* GetInstance();

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();

	private:
		static const int PCM_DATA_SIZE = 4096;

		static MusicTest* instance;

		GFX::SpriteRenderer spriteRenderer;

		u8* vorbisFileData = nullptr;
		size_t vorbisFileSize = 0;
		size_t vorbisFileOffset = 0;
		int currentVorbisBitstream = 0;

		size_t loopStart_samples = 0;
		size_t loopEnd_samples = 0;
		bool eofReached = false;

		OggVorbis_File ovFile = {};
		u8 pcmData[PCM_DATA_SIZE] = {};

		FAudio* faudio = nullptr;
		FAudioMasteringVoice* masteringVoice = nullptr;
		FAudioBuffer streamingBuffer = {};
		FAudioVoice* streamingVoice = nullptr;
		FAudioVoiceCallback streamingCallbacks = {};

		bool playing = false;
		size_t streamLocation_samples = 0;

		char debugStateString[1024] = {};

		void readLoopValues(vorbis_comment* metadata);

		static size_t callbackRead(void* ptr, size_t size, size_t nmemb, void* dataSource);
		static int callbackSeek(void* ptr, ogg_int64_t offset, int whence);
		static int callbackClose(void* dataSource);
		static long callbackTell(void* dataSource);

		static void decodeNextBuffer(FAudioVoiceCallback* callback, void* context);

		ov_callbacks vorbisCallbacks = {};
	};
}
