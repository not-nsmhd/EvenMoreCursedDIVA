#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include "../common/rect.h"
#include "../common/color.h"
#include "lowlevel/backend.h"

namespace GFX
{
	enum SpriteFlipMode : u32
	{
		FLIP_X = (1 << 0),
		FLIP_Y = (1 << 1)
	};

	struct SpriteState
	{
		vec2 position;
		vec2 origin;
		vec2 size;
		struct Common::Color colors[4];

		float rotCos;
		float rotSin;

		Common::RectangleF srcRect;
		u32 flipFlags = 0;
	};

	struct SpriteBatch
	{
		LowLevel::Texture* texture;

		std::vector<SpriteState>::const_iterator firstSprite;
		u32 spriteCount;
	};

	constexpr u32 MaxSprites = 1024;
	constexpr u32 MaxVertices = MaxSprites * 4;
	constexpr u32 MaxIndices = MaxSprites * 6;

	struct SpriteVertex
	{
		vec2 pos;
		vec2 texCoord;
		u8vec4 color;
	};

	struct SpriteUniforms
	{
		mat4 projMatrix;
	};

	class SpriteRenderer
	{
	public:
		SpriteRenderer();
		~SpriteRenderer();

		void Initialize(LowLevel::Backend* backend);
		void Destroy();

		void ResetSprite();
		void SetSpritePosition(vec2& position);
		void SetSpriteScale(vec2& absScale);
		void SetSpriteScale(const LowLevel::Texture* texture, vec2& scale);
		void SetSpriteOrigin(vec2& origin);
		void SetSpriteRotation(float radians);
		void SetSpriteSource(Common::RectangleF& source);
		void SetSpriteSource(const LowLevel::Texture* texture, Common::RectangleF& absSource);
		void SetSpriteFlip(u32 flipFlags);
		void SetSpriteColor(Common::Color color);
		
		// NOTE: Coloring order; top-left, top-right, bottom-left, bottom-right
		void SetSpriteColors(Common::Color colors[4]);
		void SetSpriteColors(Common::Color topLeft, Common::Color topRight, Common::Color bottomLeft, Common::Color bottomRight);

		void PushSprite(LowLevel::Texture* texture);

		void RenderSprites(LowLevel::Shader* shader);
	private:
		LowLevel::Backend* gfxBackend = nullptr;
		bool initialized = false;

		// NOTE: State
		std::vector<SpriteState> sprites;
		std::vector<SpriteBatch> batches;

		std::vector<SpriteState>::iterator currentSprite;
		std::vector<SpriteBatch>::iterator currentBatch;

		size_t spriteCount = 0;
		size_t batchCount = 0;

		SpriteUniforms uniforms;

		// NOTE: Buffer data
		SpriteVertex* vertexData = nullptr;

		// NOTE: Graphics data
		LowLevel::Buffer* vertexBuffer = nullptr;
		LowLevel::Buffer* indexBuffer = nullptr;
		LowLevel::Shader* defaultShader = nullptr;
		LowLevel::VertexDescription* vtxDesc = nullptr;

		LowLevel::Texture* blankTexture = {};

		// NOTE: Helper functions
		void InitializeIndexBuffer();
	};
};
