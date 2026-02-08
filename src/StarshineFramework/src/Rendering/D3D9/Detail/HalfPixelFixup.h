#pragma once
#include "Common/Types.h"
#include <memory>

namespace Starshine::Rendering::D3D9::Detail
{
	// Microsoft moment
	// References: 
	// https://learn.microsoft.com/en-us/windows/win32/direct3d9/directly-mapping-texels-to-pixels?redirectedfrom=MSDN
	// https://aras-p.info/blog/2016/04/08/solving-dx9-half-pixel-offset/
	namespace HalfPixelFixup
	{
		std::unique_ptr<u32[]> GetPatchedVertexShaderBytecode(const u8* bytecode, size_t bytecodeSize);
	}
}
