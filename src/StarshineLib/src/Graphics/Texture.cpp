#include "Texture.h"
#include <SDL2/SDL_stdinc.h>
#include "qoi.h"
#include "Misc/Compression.h"

using namespace Starshine::Misc;

namespace Starshine::Graphics
{
	namespace FileFormatDetail
	{
		static constexpr u8 CurrentRevision = 1;
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

	TextureFlags& Texture::GetFlags()
	{
		return flags;
	}

	const TextureFlags& Texture::GetFlags() const
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
		reader.ReadBuffer(&fileFlags, sizeof(fileFlags));

		format = static_cast<TextureFormat>(reader.ReadI16());
		size_t mipLevelCount = reader.ReadU16();

		size_t blockDataSize = reader.ReadSize();
		size_t compressedSize = reader.ReadSize();
		reader.Skip(8);

		dataSize = GetTextureDataSize(size, format);
		data = std::make_unique<u8[]>(dataSize);

		std::unique_ptr<u8[]> qoiData{};

		u8* readDest{};
		size_t readSize = dataSize;

		if (fileFlags.CompressedQOI)
		{
			qoiData = std::make_unique<u8[]>(blockDataSize);
			readDest = qoiData.get();
			readSize = blockDataSize;
		}
		else
			readDest = data.get();

		if (fileFlags.CompressedBZ2)
			Compression::DecompressStream(reader.GetBaseStream(), compressedSize, readDest);
		else
			reader.ReadBuffer(readDest, readSize);

		if (fileFlags.CompressedQOI)
		{
			qoi_desc desc{};
			desc.width = size.x;
			desc.height = size.y;
			desc.channels = 4;

			int qoiOutLen = {};
			u8* qoiOutData = data.get();

			qoi_decode_headerless(qoiData.get(), blockDataSize, &desc, (void**)&qoiOutData, &qoiOutLen);
		}

		return true;
	}

	void Texture::WriteBinary(IO::StreamWriter& writer)
	{
		assert(dataSize > 0 && data != nullptr);

		if (fileFlags.CompressedQOI && (format != TextureFormat::RGBA8))
			fileFlags.CompressedQOI = 0;

		writer.WriteBuffer(FileFormatDetail::FileSignature.data(), FileFormatDetail::FileSignature.size());
		writer.WriteU16(size.x);
		writer.WriteU16(size.y);

		writer.WriteBuffer(&flags, sizeof(flags)); // NOTE: Texture flags
		writer.WriteBuffer(&fileFlags, sizeof(fileFlags));

		writer.WriteI16(static_cast<i16>(format));
		writer.WriteU16(1); // NOTE: Mip level count

		size_t dataSizePos = writer.GetPosition();
		writer.WriteSize(0);

		size_t compressedSizePos = writer.GetPosition();
		writer.WriteSize(0);

		writer.WriteSize(0); // Reserved
		writer.WriteSize(0); // Reserved

		size_t dataPos = writer.GetPosition();

		u8* dataToWrite = data.get();
		size_t writeSize = dataSize;

		if (fileFlags.CompressedQOI && format == TextureFormat::RGBA8)
		{
			u8* qoiData = nullptr;
			i32 qoiDataSize = 0;

			qoi_encode_headerless(data.get(), dataSize, reinterpret_cast<void**>(&qoiData), &qoiDataSize);

			dataToWrite = qoiData;
			writeSize = qoiDataSize;
		}

		if (fileFlags.CompressedBZ2)
		{
			size_t compressedSize = Compression::CompressData(dataToWrite, writeSize, writer.GetBaseStream());
			writer.Seek(compressedSizePos);
			writer.WriteSize(compressedSize);
		}
		else
			writer.WriteBuffer(dataToWrite, writeSize);

		writer.Seek(dataSizePos);
		writer.WriteSize(writeSize);

		if (fileFlags.CompressedQOI)
			free(dataToWrite);
	}

	void Texture::SetWriteFlags(TextureFileFlags flags)
	{
		fileFlags = flags;
	}
}
