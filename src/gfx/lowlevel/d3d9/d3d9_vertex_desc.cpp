#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include <SDL2/SDL.h>
#include "d3d9_vertex_desc.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace GFXtoD3D9
		{
			D3DDECLUSAGE VertexElementType[] = 
			{
				D3DDECLUSAGE_POSITION,	// VERT_ELEMENT_POSITION
				D3DDECLUSAGE_COLOR,		// VERT_ELEMENT_COLOR
				D3DDECLUSAGE_TEXCOORD,	// VERT_ELEMENT_TEXCOORD
				D3DDECLUSAGE_NORMAL		// VERT_ELEMENT_NORMAL
			};

			D3DDECLTYPE VertexElementType_UnsignedNormalized[] =
			{
				D3DDECLTYPE_UNUSED,	// VERT_FORMAT_BYTE
				D3DDECLTYPE_UNUSED,	// VERT_FORMAT_BYTE2
				D3DDECLTYPE_UNUSED,	// VERT_FORMAT_BYTE3
				D3DDECLTYPE_UBYTE4N,	// VERT_FORMAT_BYTE4

				D3DDECLTYPE_UNUSED,	// VERT_FORMAT_SHORT
				D3DDECLTYPE_USHORT2N,	// VERT_FORMAT_SHORT2
				D3DDECLTYPE_UNUSED,	// VERT_FORMAT_SHORT3
				D3DDECLTYPE_USHORT4N,	// VERT_FORMAT_SHORT4

				D3DDECLTYPE_UNUSED,	// VERT_FORMAT_INT
				D3DDECLTYPE_UNUSED,	// VERT_FORMAT_INT2
				D3DDECLTYPE_UNUSED,	// VERT_FORMAT_INT3
				D3DDECLTYPE_UNUSED,	// VERT_FORMAT_INT4

				D3DDECLTYPE_FLOAT1,			// VERT_FORMAT_FLOAT
				D3DDECLTYPE_FLOAT2,			// VERT_FORMAT_FLOAT1
				D3DDECLTYPE_FLOAT3,			// VERT_FORMAT_FLOAT2
				D3DDECLTYPE_FLOAT4			// VERT_FORMAT_FLOAT3
			};

			UINT VertexElementType_Sizes[] =
			{
				1,	// VERT_FORMAT_BYTE
				2,	// VERT_FORMAT_BYTE2
				3,	// VERT_FORMAT_BYTE3
				4,	// VERT_FORMAT_BYTE4

				1,	// VERT_FORMAT_SHORT
				2,	// VERT_FORMAT_SHORT2
				3,	// VERT_FORMAT_SHORT3
				4,	// VERT_FORMAT_SHORT4

				1,	// VERT_FORMAT_INT
				2,	// VERT_FORMAT_INT2
				3,	// VERT_FORMAT_INT3
				4,	// VERT_FORMAT_INT4

				1,	// VERT_FORMAT_FLOAT
				2,	// VERT_FORMAT_FLOAT1
				3,	// VERT_FORMAT_FLOAT2
				4	// VERT_FORMAT_FLOAT3
			};
		}

		namespace D3D9
		{
			const VertexAttribute* VertexDescription_D3D9::GetAttributes() const
			{
				return attribs;
			}

			size_t VertexDescription_D3D9::GetAttributeCount() const
			{
				return attribCount;
			}

			size_t VertexDescription_D3D9::GetVertexStride() const
			{
				return vertexStride;
			}

			bool VertexDescription_D3D9::Initialize(IDirect3DDevice9* device, const VertexAttribute* attribs, u32 attribCount, size_t stride)
			{
				if (attribs == nullptr || attribCount == 0)
				{
					return false;
				}

				this->device = device;
				this->attribCount = attribCount;
				this->attribs = new VertexAttribute[attribCount];
				this->vertexStride = stride;

				SDL_memcpy(this->attribs, attribs, sizeof(VertexAttribute) * attribCount);

				this->vertexElements = new D3DVERTEXELEMENT9[attribCount + 1];
				vertexElements[attribCount] = D3DDECL_END();

				const VertexAttribute* currentAttrib = nullptr;
				for (int i = 0; i < attribCount; i++)
				{
					currentAttrib = &attribs[i];

					vertexElements[i].Stream = 0;
					vertexElements[i].Offset = static_cast<WORD>(currentAttrib->offset);
					vertexElements[i].Type = GFXtoD3D9::VertexElementType_UnsignedNormalized[static_cast<int>(currentAttrib->format)];
					vertexElements[i].Method = D3DDECLMETHOD_DEFAULT;
					vertexElements[i].Usage = GFXtoD3D9::VertexElementType[static_cast<int>(currentAttrib->type)];
					vertexElements[i].UsageIndex = static_cast<BYTE>(currentAttrib->typeIndex);
				}

				device->CreateVertexDeclaration(vertexElements, &vertexDecl);
				return true;
			}

			void VertexDescription_D3D9::Destroy()
			{
				delete[] attribs;
				delete[] vertexElements;
				vertexDecl->Release();
			}
			
			void VertexDescription_D3D9::Set() const
			{
				if (attribs == nullptr || attribCount == 0)
				{
					return;
				}

				device->SetVertexDeclaration(vertexDecl);
			}
		}
	}
}
#endif
#endif
