#include "FileStream.h"

namespace Starshine::IO
{
	FileStream::FileStream(FileStream&& other) : FileStream()
	{
		readable = other.readable;
		writable = other.writable;
		position = other.position;
		size = other.size;
		rwops = other.rwops;

		other.readable = false;
		other.writable = false;
		other.position = {};
		other.size = {};
		other.rwops = NULL;
	}

	FileStream::~FileStream()
	{
		Close();
	}

	bool FileStream::IsOpen() const
	{
		return rwops != NULL;
	}

	bool FileStream::IsReadable() const
	{
		return readable;
	}

	bool FileStream::IsWritable() const
	{
		return writable;
	}

	size_t FileStream::GetPosition() const
	{
		return position;
	}

	size_t FileStream::GetSize() const
	{
		return size;
	}

	size_t FileStream::Seek(size_t position)
	{
		if (rwops != NULL)
		{
			Sint64 resultPos = SDL_RWseek(rwops, static_cast<Sint64>(position), RW_SEEK_SET);
			if (resultPos != -1)
			{
				this->position = static_cast<size_t>(resultPos);
				return this->position;
			}
		}
		return 0;
	}

	size_t FileStream::ReadBuffer(void* dest, size_t size)
	{
		if (rwops != NULL)
		{
			size_t readDataSize = SDL_RWread(rwops, dest, 1, size);
			position += readDataSize;
			return readDataSize;
		}
		return 0;
	}

	size_t FileStream::WriteBuffer(const void* src, size_t size)
	{
		if (rwops != NULL)
		{
			size_t writtenDataSize = SDL_RWwrite(rwops, src, 1, size);
			position += writtenDataSize;

			if (position > (GetSize() - writtenDataSize))
			{
				this->size += writtenDataSize;
			}

			return writtenDataSize;
		}
		return 0;
	}

	void FileStream::OpenRead(std::string_view filePath)
	{
		rwops = SDL_RWFromFile(filePath.data(), "rb");

		if (rwops != NULL)
		{
			readable = true;
			size = SDL_RWsize(rwops);
		}
	}

	void FileStream::OpenReadWrite(std::string_view filePath)
	{
		rwops = SDL_RWFromFile(filePath.data(), "r+b");

		if (rwops != NULL)
		{
			readable = true;
			writable = true;
			size = SDL_RWsize(rwops);
		}
	}

	void FileStream::CreateWrite(std::string_view filePath)
	{
		rwops = SDL_RWFromFile(filePath.data(), "wb");

		if (rwops != NULL)
		{
			writable = true;
			size = SDL_RWsize(rwops);
		}
	}

	void FileStream::Close()
	{
		if (rwops != NULL)
		{
			SDL_RWclose(rwops);
			rwops = NULL;
		}
	}
}
