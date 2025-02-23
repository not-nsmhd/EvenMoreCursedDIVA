#include <SDL2/SDL.h>
#include "opengl_vertex_desc.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace GFXtoGL
		{
			GLenum VertexElementType_Signed[] =
			{
				GL_BYTE,	// VERT_FORMAT_BYTE
				GL_BYTE,	// VERT_FORMAT_BYTE2
				GL_BYTE,	// VERT_FORMAT_BYTE3
				GL_BYTE,	// VERT_FORMAT_BYTE4

				GL_SHORT,	// VERT_FORMAT_SHORT
				GL_SHORT,	// VERT_FORMAT_SHORT2
				GL_SHORT,	// VERT_FORMAT_SHORT3
				GL_SHORT,	// VERT_FORMAT_SHORT4

				GL_INT,		// VERT_FORMAT_INT
				GL_INT,		// VERT_FORMAT_INT2
				GL_INT,		// VERT_FORMAT_INT3
				GL_INT,		// VERT_FORMAT_INT4

				GL_FLOAT,	// VERT_FORMAT_FLOAT
				GL_FLOAT,	// VERT_FORMAT_FLOAT1
				GL_FLOAT,	// VERT_FORMAT_FLOAT2
				GL_FLOAT	// VERT_FORMAT_FLOAT3
			};

			GLenum VertexElementType_Unsigned[] =
			{
				GL_UNSIGNED_BYTE,	// VERT_FORMAT_BYTE
				GL_UNSIGNED_BYTE,	// VERT_FORMAT_BYTE2
				GL_UNSIGNED_BYTE,	// VERT_FORMAT_BYTE3
				GL_UNSIGNED_BYTE,	// VERT_FORMAT_BYTE4

				GL_UNSIGNED_SHORT,	// VERT_FORMAT_SHORT
				GL_UNSIGNED_SHORT,	// VERT_FORMAT_SHORT2
				GL_UNSIGNED_SHORT,	// VERT_FORMAT_SHORT3
				GL_UNSIGNED_SHORT,	// VERT_FORMAT_SHORT4

				GL_UNSIGNED_INT,	// VERT_FORMAT_INT
				GL_UNSIGNED_INT,	// VERT_FORMAT_INT2
				GL_UNSIGNED_INT,	// VERT_FORMAT_INT3
				GL_UNSIGNED_INT,	// VERT_FORMAT_INT4

				GL_FLOAT,			// VERT_FORMAT_FLOAT
				GL_FLOAT,			// VERT_FORMAT_FLOAT1
				GL_FLOAT,			// VERT_FORMAT_FLOAT2
				GL_FLOAT			// VERT_FORMAT_FLOAT3
			};

			GLint VertexElementType_Sizes[] =
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

			const GLchar* VertexShader_AttribNames[] =
			{
				"aPosition",	// VERT_ELEMENT_POSITION
				"aColor",		// VERT_ELEMENT_COLOR
				"aTexCoord",	// VERT_ELEMENT_TEXCOORD
				"aNormal"		// VERT_ELEMENT_NORMAL
			};
		}

		namespace OpenGL_ARB
		{
			const VertexAttribute* VertexDescription_OpenGL::GetAttributes() const
			{
				return attribs;
			}

			size_t VertexDescription_OpenGL::GetAttributeCount() const
			{
				return attribCount;
			}

			size_t VertexDescription_OpenGL::GetVertexStride() const
			{
				return vertexStride;
			}

			bool VertexDescription_OpenGL::Initialize(const VertexAttribute* attribs, u32 attribCount, size_t stride)
			{
				if (attribs == nullptr || attribCount == 0)
				{
					return false;
				}

				this->attribCount = attribCount;
				this->attribs = new VertexAttribute[attribCount];
				this->vertexStride = stride;

				SDL_memcpy(this->attribs, attribs, sizeof(VertexAttribute) * attribCount);

				return true;
			}

			void VertexDescription_OpenGL::Destroy()
			{
				delete[] attribs;
			}
			
			void VertexDescription_OpenGL::Set() const
			{
				if (attribs == nullptr || attribCount == 0)
				{
					return;
				}

				const VertexAttribute* currentAttrib = nullptr;
				for (int i = 0; i < attribCount; i++)
				{
					currentAttrib = &attribs[i];

					GLint attribSize = GFXtoGL::VertexElementType_Sizes[static_cast<int>(currentAttrib->format)];

					GLenum baseFormat = currentAttrib->isUnsigned ?
						GFXtoGL::VertexElementType_Unsigned[static_cast<int>(currentAttrib->format)] :
						GFXtoGL::VertexElementType_Signed[static_cast<int>(currentAttrib->format)];

					switch (currentAttrib->type)
					{
					case VertexAttributeType::VERT_ELEMENT_POSITION:
						glVertexPointer(attribSize, baseFormat, vertexStride, (void*)currentAttrib->offset);
						break;
					case VertexAttributeType::VERT_ELEMENT_COLOR:
						glColorPointer(attribSize, baseFormat, vertexStride, (void*)currentAttrib->offset);
						break;
					case VertexAttributeType::VERT_ELEMENT_TEXCOORD:
						glTexCoordPointer(attribSize, baseFormat, vertexStride, (void*)currentAttrib->offset);
						break;
					}
				}
			}
		}
	}
}
