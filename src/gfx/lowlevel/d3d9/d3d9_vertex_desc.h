#pragma once
#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include <d3d9.h>
#include "../vertex_desc.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace D3D9
		{
			class VertexDescription_D3D9 : public VertexDescription
			{
			public:
				VertexDescription_D3D9() {}

				const VertexAttribute* GetAttributes() const;
				size_t GetAttributeCount() const;
				size_t GetVertexStride() const;

				bool Initialize(IDirect3DDevice9* device, const VertexAttribute* attribs, u32 attribCount, size_t stride);
				void Destroy();

				void Set() const;
			private:
				IDirect3DDevice9* device = nullptr;

				D3DVERTEXELEMENT9* vertexElements = nullptr;
				IDirect3DVertexDeclaration9* vertexDecl = nullptr;
			};
		}
	}
}
#endif
#endif
