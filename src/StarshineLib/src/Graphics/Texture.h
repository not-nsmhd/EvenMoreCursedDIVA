#pragma once
#include <memory>
#include "Common/Types.h"
#include "Types.h"
#include "GPUResource.h"
#include "IO/StreamReader.h"
#include "IO/StreamWriter.h"

namespace Starshine::Graphics
{
	constexpr size_t MaxTextureNameLength{ 128 };

	class Texture
	{
	public:
		// NOTE: Meant to be used if the texture is created by reading a file
		Texture();
		Texture(ivec2 size, TextureFormat format, TextureFlags flags, const void* data, bool dynamic = false);
		Texture(ivec2 size, TextureFormat format, TextureFlags flags);
		~Texture() = default;

	public:
		ivec2 GetSize() const;
		TextureFormat GetFormat() const;
		TextureFlags GetFlags() const;

		u8* GetData();
		const u8* GetData() const;
		size_t GetDataSize() const;

		void SetData(ivec2 offset, ivec2 size, const u8* data);

		std::string_view GetName() const;
		void SetName(std::string_view name);

	public:
		bool ReadBinary(IO::StreamReader& writer);
		void WriteBinary(IO::StreamWriter& writer);

	public:
		ManagedGPUResource GPUTexture{};

	private:
		std::array<char, MaxTextureNameLength + 1> name; // NOTE: Additional byte for a null terminator

		ivec2 size{};
		TextureFormat format{};
		TextureFlags flags{};

		size_t dataSize{};
		std::unique_ptr<u8[]> data{};
	};
}
