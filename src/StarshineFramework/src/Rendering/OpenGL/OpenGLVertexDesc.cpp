#include "OpenGLVertexDesc.h"
#include "OpenGLCommon.h"

namespace Starshine::Rendering::OpenGL
{
	VertexDesc_OpenGL::VertexDesc_OpenGL(OpenGLDevice& device, const VertexAttrib* attribs, size_t attribCount) : DeviceRef(device)
	{
		Attribs.reserve(attribCount);
		for (size_t i = 0; i < attribCount; i++)
		{
			const VertexAttrib& gfxAttrib = attribs[i];
			VertexAttrib_OpenGL& glAttrib = Attribs.emplace_back();

			glAttrib.Type = gfxAttrib.Type;
			glAttrib.Index = gfxAttrib.Index;
			glAttrib.Format = ConversionTables::GLVertexAttribFormat[static_cast<size_t>(gfxAttrib.Format)];
			glAttrib.Components = gfxAttrib.Components;
			glAttrib.VertexSize = gfxAttrib.VertexSize;
			glAttrib.Offset = gfxAttrib.Offset;
		}
	}

	VertexDesc_OpenGL::~VertexDesc_OpenGL()
	{
		Attribs.clear();
	}
}
