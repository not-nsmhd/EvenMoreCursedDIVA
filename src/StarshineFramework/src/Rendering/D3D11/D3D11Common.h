#pragma once
#include "Rendering/Types.h"
#include <dxgiformat.h>

namespace Starshine::Rendering::D3D11
{
	namespace ConversionTables
	{
		static constexpr std::array<DXGI_FORMAT, EnumCount<GFX::TextureFormat>()> DXGITextureFormats
		{
			DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM,
			DXGI_FORMAT::DXGI_FORMAT_R8_UNORM,

			DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM,
			DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM,
			DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM
		};
	}
}
