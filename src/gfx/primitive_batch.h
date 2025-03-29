#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "../common/rect.h"
#include "../common/color.h"
#include "lowlevel/backend.h"

namespace GFX
{
	struct LineSegment
	{
		u32 vertexCount = 0;
		u32 vertexOffset = 0;

		struct Common::Color color;
	};

	struct PrimitiveVertex
	{
		glm::vec2 position;
		glm::u8vec4 color;
	};

	const u32 MAX_LINES = 1024;
	const u32 MAX_LINE_VERTICES = MAX_LINES * 2;

	class PrimitiveBatch
	{
	public:
		PrimitiveBatch();
		~PrimitiveBatch();

		void Initialize(LowLevel::Backend* backend);
		void Destroy();

		void BeginLineSegment(struct Common::Color color);
		void EndLineSegment();

		void PushLinePoint(glm::vec2 point);

		void RenderPrimitives();
	private:
		LowLevel::Backend* gfxBackend = nullptr;
		bool initialized = false;

		PrimitiveVertex* vertices = nullptr;
		size_t vertexCount = 0;

		std::vector<LineSegment> lineSegments;
		size_t lineSegmentCount = 0;

		LineSegment currentLineSegment = {};

		glm::mat4 projMatrix;

		LowLevel::Buffer* vertexBuffer = nullptr;
		LowLevel::Shader* primitiveShader = nullptr;
		LowLevel::VertexDescription* vertexDesc = nullptr;
	};
}
