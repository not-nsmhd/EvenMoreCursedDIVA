#include <SDL2/SDL.h>
#include "d3d9_defs.h"
#include "d3d9_vertex_desc.h"

namespace GFX::LowLevel::D3D9
{
	namespace GFXtoD3D9
	{
		D3DDECLTYPE VertexElementType[] =
		{
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_BYTE
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_BYTE2
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_BYTE3
				D3DDECLTYPE::D3DDECLTYPE_UBYTE4, // VERT_FORMAT_BYTE4

				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_SHORT
				D3DDECLTYPE::D3DDECLTYPE_SHORT2, // VERT_FORMAT_SHORT2
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_SHORT3
				D3DDECLTYPE::D3DDECLTYPE_SHORT4, // VERT_FORMAT_SHORT4

				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_INT
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_INT2
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_INT3
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_INT4

				D3DDECLTYPE::D3DDECLTYPE_FLOAT1, // VERT_FORMAT_FLOAT
				D3DDECLTYPE::D3DDECLTYPE_FLOAT2, // VERT_FORMAT_FLOAT1
				D3DDECLTYPE::D3DDECLTYPE_FLOAT3, // VERT_FORMAT_FLOAT2
				D3DDECLTYPE::D3DDECLTYPE_FLOAT4  // VERT_FORMAT_FLOAT3
		};

		D3DDECLTYPE VertexElementType_Normalized[] =
		{
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_BYTE
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_BYTE2
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_BYTE3
				D3DDECLTYPE::D3DDECLTYPE_UBYTE4N, // VERT_FORMAT_BYTE4

				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_SHORT
				D3DDECLTYPE::D3DDECLTYPE_SHORT2N, // VERT_FORMAT_SHORT2
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_SHORT3
				D3DDECLTYPE::D3DDECLTYPE_SHORT4N, // VERT_FORMAT_SHORT4

				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_INT
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_INT2
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_INT3
				D3DDECLTYPE::D3DDECLTYPE_UNUSED, // VERT_FORMAT_INT4

				D3DDECLTYPE::D3DDECLTYPE_FLOAT1, // VERT_FORMAT_FLOAT
				D3DDECLTYPE::D3DDECLTYPE_FLOAT2, // VERT_FORMAT_FLOAT1
				D3DDECLTYPE::D3DDECLTYPE_FLOAT3, // VERT_FORMAT_FLOAT2
				D3DDECLTYPE::D3DDECLTYPE_FLOAT4  // VERT_FORMAT_FLOAT3
		};

		WORD VertexElementType_Sizes[] =
			{
				1, // VERT_FORMAT_BYTE
				2, // VERT_FORMAT_BYTE2
				3, // VERT_FORMAT_BYTE3
				4, // VERT_FORMAT_BYTE4

				1, // VERT_FORMAT_SHORT
				2, // VERT_FORMAT_SHORT2
				3, // VERT_FORMAT_SHORT3
				4, // VERT_FORMAT_SHORT4

				1, // VERT_FORMAT_INT
				2, // VERT_FORMAT_INT2
				3, // VERT_FORMAT_INT3
				4, // VERT_FORMAT_INT4

				1, // VERT_FORMAT_FLOAT
				2, // VERT_FORMAT_FLOAT1
				3, // VERT_FORMAT_FLOAT2
				4  // VERT_FORMAT_FLOAT3
		};

		D3DDECLUSAGE VertexElementType_Usage[] =
			{
				D3DDECLUSAGE::D3DDECLUSAGE_POSITION, // VERT_ELEMENT_POSITION
				D3DDECLUSAGE::D3DDECLUSAGE_COLOR,	 // VERT_ELEMENT_COLOR
				D3DDECLUSAGE::D3DDECLUSAGE_TEXCOORD, // VERT_ELEMENT_TEXCOORD
				D3DDECLUSAGE::D3DDECLUSAGE_NORMAL	 // VERT_ELEMENT_NORMAL
		};
	}

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

	bool VertexDescription_D3D9::Initialize(const VertexAttribute *attribs, u32 attribCount, size_t stride)
	{
		if (attribs == nullptr || attribCount == 0)
		{
			return false;
		}

		this->attribCount = attribCount;
		this->attribs = new VertexAttribute[attribCount];
		this->vertexStride = stride;

		D3DVERTEXELEMENT9* d3dVertexElements = new D3DVERTEXELEMENT9[attribCount + 1];

		const VertexAttribute* attrib = nullptr;
		for (u32 i = 0; i < attribCount; i++)
		{
			attrib = &attribs[i];

			d3dVertexElements[i].Stream = 0;
			d3dVertexElements[i].Offset = static_cast<WORD>(attrib->offset);
			d3dVertexElements[i].Type = attrib->normalized ? GFXtoD3D9::VertexElementType_Normalized[static_cast<int>(attrib->format)] :
			GFXtoD3D9::VertexElementType[static_cast<int>(attrib->format)];
			d3dVertexElements[i].Method = D3DDECLMETHOD::D3DDECLMETHOD_DEFAULT;
			d3dVertexElements[i].Usage = GFXtoD3D9::VertexElementType_Usage[static_cast<int>(attrib->type)];
			d3dVertexElements[i].UsageIndex = static_cast<BYTE>(attrib->typeIndex);
		}

		d3dVertexElements[attribCount] = D3DDECL_END();

		HRESULT result = device->CreateVertexDeclaration(d3dVertexElements, &vertexDecl);
		if (result != D3D_OK)
		{
			LOG_ERROR_ARGS("Failed to create a vertex description. Error: 0x%X", result);
			delete[] d3dVertexElements;
			return false;
		}

		LOG_INFO_ARGS("Created a new vertex description at address 0x%X", vertexDecl);
		delete[] d3dVertexElements;
		return true;
	}

	void VertexDescription_D3D9::Destroy()
	{
		if (attribs != nullptr)
		{
			delete[] attribs;
			attribs = nullptr;

			vertexDecl->Release();

			if (nameSet)
			{
				LOG_INFO_ARGS("Destroyed a vertex description \"%s\"", name);
			}
			else
			{
				LOG_INFO_ARGS("Destroyed a vertex description at address 0x%X", vertexDecl);
			}

			vertexDecl = NULL;
		}
	}

	void VertexDescription_D3D9::Set()
	{
		if (attribs == nullptr || attribCount == 0)
		{
			return;
		}

		device->SetVertexDeclaration(vertexDecl);
	}
}
