#include <fstream>
#include <SDL2/SDL.h>
#include "io/filesystem.h"
#include "music.h"

using std::fstream;
using std::ios;

namespace Audio
{
	static const size_t PCM_DATA_BYTES = 4096;

	namespace VorbisfileCallbacks
	{
		size_t read(void* dst, size_t size, size_t nmemb, void* musicPtr)
		{
			if (musicPtr == nullptr)
			{
				return 0;
			}

			Music* music = static_cast<Music*>(musicPtr);

			size_t copySize = size * nmemb;
			return music->ReadVorbisData(dst, copySize);
		}

		int seek(void* musicPtr, ogg_int64_t offset, int whence)
		{
			if (musicPtr == nullptr)
			{
				return 0;
			}

			Music* music = static_cast<Music*>(musicPtr);
			return music->SeekVorbisData(offset, whence);
		}

		int close(void* musicPtr)
		{
			return 0;
		}

		long tell(void* musicPtr)
		{
			if (musicPtr == nullptr)
			{
				return 0;
			}

			Music* music = static_cast<Music*>(musicPtr);
			return music->GetVorbisDataOffset();
		}
	}

	Music::Music()
	{
	}
	
	Music::~Music()
	{
	}
	
	bool Music::LoadFromFile(const std::filesystem::path& path)
	{
		IO::FileSystem* fs = IO::FileSystem::GetInstance();
		std::filesystem::path fullPath = fs->GetContentFilePath(path);

		if (fullPath.empty())
		{
			return false;
		}

		fstream vorbisFile = fstream(fullPath, ios::in | ios::binary);
		if (vorbisFile.bad())
		{
			vorbisFile.close();
			return false;
		}

		vorbisFile.seekg(0, ios::end);
		vorbisFileSize = vorbisFile.tellg();
		vorbisFile.seekg(0, ios::beg);

		vorbisFileData = new u8[vorbisFileSize];
		vorbisFile.read((char*)vorbisFileData, vorbisFileSize);
		vorbisFile.close();

		ov_callbacks vorbisCallbacks = {};
		vorbisCallbacks.read_func = (&VorbisfileCallbacks::read);
		vorbisCallbacks.seek_func = (&VorbisfileCallbacks::seek);
		vorbisCallbacks.close_func = NULL;
		vorbisCallbacks.tell_func = (&VorbisfileCallbacks::tell);
		int res = ov_open_callbacks(this, &ovFile, NULL, 0, vorbisCallbacks);

		vorbis_info* ovInfo = ov_info(&ovFile, -1);
		channels = ovInfo->channels;
		sampleRate = ovInfo->rate;

		pcmData = new u8[PCM_DATA_BYTES];
		pcmDataSize_bytes = PCM_DATA_BYTES;
		return true;
	}
	
	void Music::Destroy()
	{
		if (vorbisFileData != nullptr)
		{
			ov_clear(&ovFile);
			
			delete[] vorbisFileData;
			vorbisFileData = nullptr;

			vorbisFileSize = 0;
			vorbisFileOffset = 0;
			vorbisBitstream = 0;
		}

		if (pcmData != nullptr)
		{
			delete[] pcmData;
			pcmData = nullptr;

			pcmDataSize_bytes = 0;
			decodedPcmDataSize_bytes = 0;
			streamLocation_samples = 0;
			eofReached = false;
		}
	}
	
	size_t Music::ReadVorbisData(void* dst, size_t size)
	{
		if (dst == nullptr || size == 0)
		{
			return 0;
		}

		size_t copySize = SDL_min(size, vorbisFileSize - vorbisFileOffset);

		SDL_memcpy(dst, vorbisFileData + vorbisFileOffset, copySize);
		vorbisFileOffset += copySize;

		return copySize;
	}
	
	int Music::SeekVorbisData(ogg_int64_t offset, int dir)
	{
		switch (dir)
		{
			case SEEK_SET:
				vorbisFileOffset = static_cast<size_t>(offset);
				break;
			case SEEK_CUR:
				vorbisFileOffset += static_cast<size_t>(offset);
				break;
			case SEEK_END:
				vorbisFileOffset = vorbisFileSize - static_cast<size_t>(offset) - 1;
				break;
		}

		if (vorbisFileOffset >= vorbisFileSize)
		{
			vorbisFileOffset = 0;
			return OV_EINVAL;
		}

		return 0;
	}
	
	long Music::GetVorbisDataOffset()
	{
		return static_cast<long>(vorbisFileOffset);
	}
	
	void Music::Restart()
	{
		eofReached = 0;
		decodedPcmDataSize_bytes = 0;
		ov_pcm_seek(&ovFile, 0);
	}
	
	u8* Music::GetCurrentPCMBlock(size_t* outSize)
	{
		if (outSize != nullptr)
		{
			*outSize = decodedPcmDataSize_bytes;
		}

		return pcmData;
	}
	
	u8* Music::DecodeNextPCMBlock(size_t* outSize, int* lastBlock)
	{
		if (!eofReached)
		{
			long ret = ov_read(&ovFile, (char*)pcmData, PCM_DATA_BYTES, 0, 2, 1, &vorbisBitstream);

			if (ret == 0)
			{
				eofReached = true;
			}
			else if (ret < 0)
			{
			}
			else
			{
				decodedPcmDataSize_bytes = ret;

				if (outSize != nullptr)
				{
					*outSize = decodedPcmDataSize_bytes;
				}

				if (lastBlock != nullptr)
				{
					*lastBlock = (eofReached == true);
				}

				return pcmData;
			}
		}

		return nullptr;
	}
}
