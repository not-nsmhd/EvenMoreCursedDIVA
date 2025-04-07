#pragma once
#include <string>
#include "../game.h"
#include "../gfx/lowlevel/backend.h"
#include "../common/int_types.h"

namespace Testing
{
	constexpr GFX::LowLevel::VertexAttribute TestVertexAttribs[3] = 
	{
		{ GFX::LowLevel::VertexAttributeType::POSITION, 0, GFX::LowLevel::VertexAttributeFormat::VERT_FORMAT_FLOAT2, false, false, 0 },
		{ GFX::LowLevel::VertexAttributeType::COLOR, 0, GFX::LowLevel::VertexAttributeFormat::VERT_FORMAT_BYTE4, true, true, 8 },
		{ GFX::LowLevel::VertexAttributeType::TEXCOORD, 0, GFX::LowLevel::VertexAttributeFormat::VERT_FORMAT_FLOAT2, false, false, 12 }
	};

	class GFXBackendTest : public GameState
	{
	public:
		GFXBackendTest() {}

		bool Initialize();
		bool LoadContent();
		void UnloadContent();
		void Destroy();
		void OnResize(u32 newWidth, u32 newHeight);
		void Update();
		void Draw();
	private:
		GFX::LowLevel::BlendState noBlend = {};
		mat4 projMatrix = {};

		GFX::LowLevel::Buffer* vertexBuffer = nullptr;
		GFX::LowLevel::Buffer* indexBuffer = nullptr;
		GFX::LowLevel::VertexDescription* vertexDesc = nullptr;
		GFX::LowLevel::Shader* shader = nullptr;
		GFX::LowLevel::Texture* texture = nullptr;
	};
}
