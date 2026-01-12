#pragma once
#include "Common/Types.h"
#include <SDL2/SDL_endian.h>

namespace Starshine::Misc
{
	inline u16 ConvertU16ToLE(u16 value) { return SDL_SwapLE16(value); }
	inline u32 ConvertU32ToLE(u32 value) { return SDL_SwapLE32(value); }
	inline u64 ConvertU64ToLE(u64 value) { return SDL_SwapLE64(value); }

	inline i16 ConvertI16ToLE(i16 value) { return SDL_SwapLE16(value); }
	inline i32 ConvertI32ToLE(i32 value) { return SDL_SwapLE32(value); }
	inline i64 ConvertI64ToLE(i64 value) { return SDL_SwapLE64(value); }

	inline f32 ConvertF32ToLE(f32 value) { return SDL_SwapFloatLE(value); }
	inline f64 ConvertF64ToLE(f64 value) { return SDL_SwapFloatLE(value); }

	inline u16 ConvertU16ToBE(u16 value) { return SDL_SwapBE16(value); }
	inline u32 ConvertU32ToBE(u32 value) { return SDL_SwapBE32(value); }
	inline u64 ConvertU64ToBE(u64 value) { return SDL_SwapBE64(value); }
						   								  
	inline i16 ConvertI16ToBE(i16 value) { return SDL_SwapBE16(value); }
	inline i32 ConvertI32ToBE(i32 value) { return SDL_SwapBE32(value); }
	inline i64 ConvertI64ToBE(i64 value) { return SDL_SwapBE64(value); }

	inline f32 ConvertF32ToBE(f32 value) { return SDL_SwapFloatBE(value); }
	inline f64 ConvertF64ToBE(f64 value) { return SDL_SwapFloatBE(value); }
}
