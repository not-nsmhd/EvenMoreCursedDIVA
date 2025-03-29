#include <SDL2/SDL.h>
#include <glm/ext.hpp>
#include "../common/math_ext.h"
#include "helpers/shader_helpers.h"
#include "primitive_batch.h"

using std::vector;
using glm::vec2;
using glm::u8vec4;
using glm::mat4;
using Common::Color;
using namespace GFX::LowLevel;
using namespace GFX::Helpers;

namespace GFX
{
	PrimitiveBatch::PrimitiveBatch()
	{
	}
	
	PrimitiveBatch::~PrimitiveBatch()
	{
	}
	
	void PrimitiveBatch::Initialize(LowLevel::Backend* backend)
	{
		if (initialized)
		{
			return;
		}

		gfxBackend = backend;

		primitiveShader = LoadShaderFromDescriptor(gfxBackend, "shaders/Simple1.xml");

		VertexAttribute vtxAttribs[] =
		{
			{ VertexAttributeType::VERT_ELEMENT_POSITION, 0, VertexAttributeFormat::VERT_FORMAT_FLOAT2, false, false, offsetof(PrimitiveVertex, position) },
			{ VertexAttributeType::VERT_ELEMENT_COLOR, 0, VertexAttributeFormat::VERT_FORMAT_BYTE4, true, true, offsetof(PrimitiveVertex, color) }
		};

		vertexBuffer = gfxBackend->CreateVertexBuffer(BufferUsage::BUFFER_USAGE_DYNAMIC, nullptr, MAX_LINE_VERTICES * sizeof(PrimitiveVertex));
		vertexDesc = gfxBackend->CreateVertexDescription(vtxAttribs, 2, sizeof(PrimitiveVertex), primitiveShader);

		vertices = new PrimitiveVertex[MAX_LINE_VERTICES];

		initialized = true;
	}
	
	void PrimitiveBatch::Destroy()
	{
		if (initialized)
		{
			gfxBackend->DestroyBuffer(vertexBuffer);
			gfxBackend->DestroyVertexDescription(vertexDesc);
			gfxBackend->DestroyShader(primitiveShader);

			delete[] vertices;

			lineSegments.clear();
			initialized = false;
		}	
	}
	
	void PrimitiveBatch::BeginLineSegment(struct Common::Color color)
	{
		currentLineSegment.color = color;
		currentLineSegment.vertexCount = 0;
		currentLineSegment.vertexOffset = vertexCount;
	}
	
	void PrimitiveBatch::EndLineSegment()
	{
		if (currentLineSegment.vertexCount > 0)
		{
			//lineSegments.push_back(currentLineSegment);
		}
	}
	
	void PrimitiveBatch::PushLinePoint(glm::vec2 point)
	{
		if (vertexCount >= MAX_LINE_VERTICES)
		{
			EndLineSegment();
			RenderPrimitives();
		}

		vertices[vertexCount].position = point;
		vertices[vertexCount].color = u8vec4(currentLineSegment.color.R, currentLineSegment.color.G, currentLineSegment.color.B, currentLineSegment.color.A);

		currentLineSegment.vertexCount++;
		vertexCount++;
	}
	
	void PrimitiveBatch::RenderPrimitives()
	{
		if (vertexCount > 0)
		{
			gfxBackend->SetBufferData(vertexBuffer, vertices, 0, sizeof(PrimitiveVertex) * vertexCount);

			gfxBackend->BindShader(primitiveShader);
			gfxBackend->SetVertexDescription(vertexDesc);
			gfxBackend->BindVertexBuffer(vertexBuffer);

			float x;
			float y;
			float width;
			float height;
			gfxBackend->GetViewportSize(&x, &y, &width, &height);
			projMatrix = glm::orthoLH_ZO(0.0f, width, height, 0.0f, 0.0f, 1.0f);
			gfxBackend->SetShaderMatrix(0, glm::value_ptr(projMatrix));

			gfxBackend->DrawArrays(PrimitiveType::PRIMITIVE_LINE_STRIP, 0, vertexCount);

			vertexCount = 0;
		}
	}
}
