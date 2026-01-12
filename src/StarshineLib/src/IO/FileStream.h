#pragma once
#include "IStream.h"
#include <SDL2/SDL_rwops.h>

namespace Starshine::IO
{
	class FileStream final : public IStream, NonCopyable
	{
	public:
		FileStream() = default;
		FileStream(FileStream&& other);
		~FileStream();

	public:
		bool IsOpen() const override;
		bool IsReadable() const override;
		bool IsWritable() const override;

		size_t GetPosition() const override;
		size_t GetSize() const override;
		size_t Seek(size_t position) override;

		size_t ReadBuffer(void* dest, size_t size) override;
		size_t WriteBuffer(const void* src, size_t size) override;

	public:
		void OpenRead(std::string_view filePath);
		void OpenReadWrite(std::string_view filePath);
		void CreateWrite(std::string_view filePath);
		void Close() override;

	private:
		bool readable{ false };
		bool writable{ false };
		size_t position{};
		size_t size{};

		SDL_RWops* rwops{ NULL };
	};
}
