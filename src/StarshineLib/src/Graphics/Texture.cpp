#include "Texture.h"
#include <SDL2/SDL_stdinc.h>

namespace Starshine::Graphics
{
	namespace FileFormatDetail
	{
		static constexpr u8 CurrentRevision = 0;
		static constexpr std::array<char, 4> FileSignature = { 'S', 'T', 'X', CurrentRevision };
	}

	Texture::Texture()
	{
	}

	Texture::Texture(ivec2 size, TextureFormat format, TextureFlags flags, const void* initialData, bool dynamic)
		: size(size), format(format), flags(flags), name{}
	{
		if (!dynamic)
			assert(initialData != nullptr);

		GPUTexture.Dynamic = dynamic;
		dataSize = GetTextureDataSize(size, format);
		data = std::make_unique<u8[]>(dataSize);

		if (data != nullptr)
			SDL_memcpy(data.get(), initialData, dataSize);
	}

	Texture::Texture(ivec2 size, TextureFormat format, TextureFlags flags) : Texture(size, format, flags, nullptr, true)
	{
		SDL_memset(data.get(), 0, dataSize);
	}

	ivec2 Texture::GetSize() const
	{
		return size;
	}

	TextureFormat Texture::GetFormat() const
	{
		return format;
	}

	TextureFlags Texture::GetFlags() const
	{
		return flags;
	}

	u8* Texture::GetData()
	{
		return data.get();
	}

	const u8* Texture::GetData() const
	{
		return data.get();
	}

	size_t Texture::GetDataSize() const
	{
		return dataSize;
	}

	void Texture::SetData(ivec2 offset, ivec2 size, const u8* data)
	{
		// TODO: Implement
		//assert(GPUTexture.Dynamic == true);
		assert(false);
	}

	std::string_view Texture::GetName() const
	{
		return name.data();
	}

	void Texture::SetName(std::string_view name)
	{
		assert(name.size() <= MaxTextureNameLength);
		SDL_strlcpy(this->name.data(), name.data(), MaxTextureNameLength);
	}

	bool Texture::ReadBinary(IO::StreamReader& reader)
	{
		assert(data == nullptr);
		char signature[4]{};
		reader.ReadBuffer(signature, FileFormatDetail::FileSignature.size());
		if (SDL_memcmp(signature, FileFormatDetail::FileSignature.data(), FileFormatDetail::FileSignature.size()) != 0)
			return false;

		size.x = reader.ReadU16();
		size.y = reader.ReadU16();

		reader.ReadBuffer(&flags, sizeof(flags));
		u16 formatFlags = reader.ReadU16();

		format = static_cast<TextureFormat>(reader.ReadI16());
		size_t mipLevelCount = reader.ReadU16();

		dataSize = reader.ReadSize();
		data = std::make_unique<u8[]>(dataSize);
		reader.ReadBuffer(data.get(), dataSize);
			
		return true;
	}

	void Texture::WriteBinary(IO::StreamWriter& writer)
	{
		assert(dataSize > 0 && data != nullptr);

		writer.WriteBuffer(FileFormatDetail::FileSignature.data(), FileFormatDetail::FileSignature.size());
		writer.WriteU16(size.x);
		writer.WriteU16(size.y);

		writer.WriteBuffer(&flags, sizeof(flags));
		writer.WriteU16(0); // NOTE/TODO: File format flags

		writer.WriteI16(static_cast<i16>(format));
		writer.WriteU16(1); // NOTE: Mip level count

		writer.WriteSize(dataSize);
		writer.WriteBuffer(data.get(), dataSize);
	}
}
