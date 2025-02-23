#pragma once
#include "../../common/int_types.h"

namespace GFX
{
	namespace LowLevel
	{
		enum class TextureFormat
		{
			TEXFORMAT_UNKNOWN = -1,
			TEXFORMAT_RGBA8,
			TEXFORMAT_RGB565,
			TEXFORMAT_RGB5_A1,
			TEXFORMAT_RGBA4,
			TEXFORMAT_R8
		};

		enum TextureFlags
		{
			GFX_TEXFLAG_NONE = 0,

			GFX_TEXFLAG_NEAREST_FILTER = 1,
			GFX_TEXFLAG_CLAMP_S = 2,
			GFX_TEXFLAG_CLAMP_T = 4
		};

		class Texture
		{
		public:
			Texture() {}

			virtual const char* GetDebugName() const = 0;
			virtual void SetDebugName(const char* name) = 0;

			virtual u32 GetWidth() const = 0;
			virtual u32 GetHeight() const = 0;
			virtual TextureFormat GetFormat() const = 0;
		protected:
			char* debugName = nullptr;

			u32 width = 0;
			u32 height = 0;
			TextureFormat format = TextureFormat::TEXFORMAT_UNKNOWN;
			u32 flags = 0;
		};
	}
}
