#pragma once
#include "Rendering/Types.h"
#include <d3d9.h>

namespace Starshine::Rendering::D3D9
{
	namespace ConversionTables
	{
		constexpr std::array<D3DPRIMITIVETYPE, EnumCount<PrimitiveType>()> D3DPrimitiveTypes =
		{
			D3DPT_POINTLIST,
			D3DPT_LINELIST,
			D3DPT_LINESTRIP,
			D3DPT_TRIANGLELIST,
			D3DPT_TRIANGLESTRIP
		};

		constexpr std::array<u32, EnumCount<PrimitiveType>()> D3DVerticesPerPrimitive =
		{
			1,
			2,
			2,
			3,
			1
		};

		constexpr std::array<D3DFORMAT, EnumCount<IndexFormat>()> D3DIndexFormats =
		{
			D3DFMT_INDEX16,
			D3DFMT_INDEX32
		};

		constexpr std::array<D3DBLEND, EnumCount<BlendFactor>()> D3DBlendFactors =
		{
			D3DBLEND_ZERO,
			D3DBLEND_ONE,
			D3DBLEND_SRCCOLOR,
			D3DBLEND_INVSRCCOLOR,
			D3DBLEND_DESTCOLOR,
			D3DBLEND_INVDESTCOLOR,
			D3DBLEND_SRCALPHA,
			D3DBLEND_INVSRCALPHA,
			D3DBLEND_DESTALPHA,
			D3DBLEND_INVDESTALPHA
		};

		constexpr std::array<D3DBLENDOP, EnumCount<BlendOperation>()> D3DBlendOperations =
		{
			D3DBLENDOP_ADD,
			D3DBLENDOP_SUBTRACT,
			D3DBLENDOP_REVSUBTRACT,
			D3DBLENDOP_MIN,
			D3DBLENDOP_MAX
		};

		constexpr std::array<D3DCULL, EnumCount<PolygonOrientation>()> D3DPolygonOrientation =
		{
			D3DCULL_CW,
			D3DCULL_CCW
		};

		constexpr std::array<D3DFORMAT, EnumCount<GFX::TextureFormat>()> D3DTextureDataFormats =
		{
			D3DFMT_A8R8G8B8,
			D3DFMT_A8L8,
			D3DFMT_A8,

			D3DFMT_DXT1,
			D3DFMT_DXT3,
			D3DFMT_DXT5
		};

		constexpr std::array<u32, EnumCount<GFX::TextureFormat>()> D3DBytesPerPixel =
		{
			4,
			2,
			1,

			0,
			0,
			0
		};
	}
}
