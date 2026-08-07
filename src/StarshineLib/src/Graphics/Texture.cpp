#include "Texture.h"
#include <SDL2/SDL_stdinc.h>

namespace Starshine::Graphics
{
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
}
