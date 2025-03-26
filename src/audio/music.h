#pragma once
#include <filesystem>
#include <vorbis/vorbisfile.h>
#include "common/int_types.h"

namespace Audio
{
	class Music
	{
	public:
		Music();
		~Music();

		bool LoadFromFile(const std::filesystem::path& path);
		void Destroy();

		size_t ReadVorbisData(void* dst, size_t size);
		int SeekVorbisData(ogg_int64_t offset, int dir);
		long GetVorbisDataOffset();

		void Restart();

		u8* GetCurrentPCMBlock(size_t* outSize);
		u8* DecodeNextPCMBlock(size_t* outSize, int* lastBlock);
	private:
		u8* vorbisFileData = nullptr;
		size_t vorbisFileSize = 0;
		size_t vorbisFileOffset = 0;
		int vorbisBitstream = 0;

		OggVorbis_File ovFile = {};

		size_t channels = 0;
		size_t sampleRate = 0;

		u8* pcmData = nullptr;
		size_t pcmDataSize_bytes = 0;
		size_t decodedPcmDataSize_bytes = 0;

		size_t streamLocation_samples = 0;
		bool eofReached = false;
	};
};
