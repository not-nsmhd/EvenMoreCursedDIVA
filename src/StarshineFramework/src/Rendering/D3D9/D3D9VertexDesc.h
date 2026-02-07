#pragma once
#include "D3D9Device.h"
#include <d3d9.h>

namespace Starshine::Rendering::D3D9
{
	struct VertexDesc_D3D9 : public VertexDesc
	{
		VertexDesc_D3D9(IDirect3DDevice9* device, const VertexAttrib* attribs, size_t attribCount);
		~VertexDesc_D3D9();

		IDirect3DDevice9* Device{};
		IDirect3DVertexDeclaration9* BaseDeclaration{};
		UINT VertexStride{};
	};
}
