#include <fstream>
#include <SDL2/SDL.h>
#include "helpers.h"
#include "../io/filesystem.h"

using std::fstream;
using std::ios;
using namespace IO;

namespace Audio
{
	namespace Helpers
	{
		bool LoadSoundEffect(SoundEffect* output, const std::filesystem::path& path)
		{
			if (path.empty() || output == nullptr)
			{
				return false;
			}

			FileSystem* fs = FileSystem::GetInstance();
			std::filesystem::path fullPath = fs->GetContentFilePath(path);
			
			if (fullPath.empty())
			{
				return false;
			}

			std::filesystem::path fileExt = fullPath.extension();
			size_t fileSize = std::filesystem::file_size(fullPath);

			if (fileExt == ".wav")
			{
				fstream wavFile = fstream(fullPath, ios::in | ios::binary);

				if (wavFile.bad())
				{
					return false;
				}

				u8* wavFileData = new u8[fileSize];
				wavFile.read((char*)wavFileData, fileSize);

				wavFile.close();

				SDL_RWops* wavFileRW = SDL_RWFromConstMem(wavFileData, fileSize);
				SDL_AudioSpec audioSpec = {};
				u8* audioData = NULL;
				u32 audioDataSize = 0;

				if (SDL_LoadWAV_RW(wavFileRW, 0, &audioSpec, &audioData, &audioDataSize) == NULL)
				{
					SDL_RWclose(wavFileRW);
					delete[] wavFileData;
					return false;
				}

				SDL_RWclose(wavFileRW);
				delete[] wavFileData;

				if (audioSpec.channels != 2 || audioSpec.format != AUDIO_S16 || audioSpec.freq != 44100)
				{
					SDL_FreeWAV(audioData);
					return false;
				}

				output->Create(2, 44100, audioDataSize);
				SDL_memcpy(output->GetPCMData(), audioData, audioDataSize);

				SDL_FreeWAV(audioData);
			}
			else if (fileExt == ".ogg")
			{
				// TODO: Implement
			}
			else
			{
				return false;
			}

			return true;
		}
	}
}
