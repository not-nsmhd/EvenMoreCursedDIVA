#include "D3D9VertexDesc.h"
#include "D3D9Common.h"
#include "Common/MathExt.h"
#include <vector>

namespace Starshine::Rendering::D3D9
{
	namespace ConversionTables
	{
		static constexpr std::array<D3DDECLTYPE, EnumCount<VertexAttribFormat>()> ElementFormat
		{
			D3DDECLTYPE_FLOAT1,
			D3DDECLTYPE_FLOAT2,
			D3DDECLTYPE_FLOAT3,
			D3DDECLTYPE_FLOAT4,
			
			D3DDECLTYPE_UBYTE4,
			D3DDECLTYPE_UBYTE4N
		};

		static constexpr std::array<D3DDECLUSAGE, EnumCount<VertexAttribType>()> ElementUsage
		{
			D3DDECLUSAGE_POSITION,
			D3DDECLUSAGE_COLOR,
			D3DDECLUSAGE_TEXCOORD
		};
	}

	VertexDesc_D3D9::VertexDesc_D3D9(IDirect3DDevice9* device, const VertexAttrib* attribs, size_t attribCount) : Device(device)
	{
		std::vector<D3DVERTEXELEMENT9> d3dAttribs;
		d3dAttribs.reserve(attribCount + 1);

		for (size_t i = 0; i < attribCount; i++)
		{
			const VertexAttrib& gfxAttrib = attribs[i];
			D3DVERTEXELEMENT9& element = d3dAttribs.emplace_back();

			element.Stream = 0;
			element.Offset = static_cast<WORD>(gfxAttrib.Offset);
			element.Type = ConversionTables::ElementFormat[static_cast<size_t>(gfxAttrib.Format)];
			element.Method = D3DDECLMETHOD_DEFAULT;
			element.Usage = ConversionTables::ElementUsage[static_cast<size_t>(gfxAttrib.Type)];
			element.UsageIndex = static_cast<BYTE>(gfxAttrib.Index);
			VertexStride = MathExtensions::Max<SHORT>(gfxAttrib.VertexSize, VertexStride);
		}

		d3dAttribs.push_back(D3DDECL_END());

		Device->CreateVertexDeclaration(d3dAttribs.data(), &BaseDeclaration);
	}

	VertexDesc_D3D9::~VertexDesc_D3D9()
	{
		BaseDeclaration->Release();
	}
}
