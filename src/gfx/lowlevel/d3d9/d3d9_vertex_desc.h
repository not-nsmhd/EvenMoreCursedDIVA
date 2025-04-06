#pragma once
#include <d3d9.h>
#include "../vertex_desc.h"

namespace GFX::LowLevel::D3D9
{
	class VertexDescription_D3D9 : public VertexDescription
	{
	public:
		VertexDescription_D3D9() = delete;
		VertexDescription_D3D9(LPDIRECT3DDEVICE9 d3dDevice) : device(d3dDevice) {}

		const VertexAttribute *GetAttributes() const;
		size_t GetAttributeCount() const;
		size_t GetVertexStride() const;

		bool Initialize(const VertexAttribute *attribs, u32 attribCount, size_t stride);
		void Destroy();

		void Set();
	private:
		LPDIRECT3DDEVICE9 device = NULL;

		LPDIRECT3DVERTEXDECLARATION9 vertexDecl = NULL;
	};
}
