#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include "../common/rect.h"
#include "../common/color.h"
#include "lowlevel/backend.h"

using std::vector;
using glm::vec2;
using glm::u8vec4;
using glm::mat4;
using Common::RectangleF;
using Common::Color;

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
		struct Color colors[4];

		float rotCos;
		float rotSin;

		RectangleF srcRect;
		u32 flipFlags = 0;
	};

	struct SpriteBatch
	{
		const LowLevel::Texture* texture;

		vector<SpriteState>::iterator firstSprite;
		u32 spriteCount;
	};

	const u32 MAX_SPRITES = 1024;
	const u32 MAX_VERTICES = MAX_SPRITES * 4;
	const u32 MAX_INDICES = MAX_SPRITES * 6;

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
		void SetSpritePosition(vec2 position);
		void SetSpriteScale(vec2 absScale);
		void SetSpriteScale(const LowLevel::Texture* texture, vec2 scale);
		void SetSpriteOrigin(vec2 origin);
		void SetSpriteRotation(float radians);
		void SetSpriteSource(RectangleF source);
		void SetSpriteSource(const LowLevel::Texture* texture, RectangleF absSource);
		void SetSpriteFlip(u32 flipFlags);
		void SetSpriteColor(struct Color color);
		
		// Coloring order; top-left, top-right, bottom-left, bottom-right
		void SetSpriteColors(struct Color colors[4]);

		void PushSprite(const LowLevel::Texture* texture);

		void RenderSprites(const LowLevel::Shader* shader);
	private:
		LowLevel::Backend* gfxBackend = nullptr;
		bool initialized = false;

		/* State */

		vector<SpriteState> sprites;
		vector<SpriteBatch> batches;

		vector<SpriteState>::iterator currentSprite;
		vector<SpriteBatch>::iterator currentBatch;

		size_t spriteCount = 0;
		size_t batchCount = 0;

		/* Uniforms */

		SpriteUniforms uniforms;

		/* Buffer data */

		SpriteVertex* vertexData = nullptr;

		/* Graphical resources */

		LowLevel::Buffer* vertexBuffer = nullptr;
		LowLevel::Buffer* indexBuffer = nullptr;
		LowLevel::Shader* defaultShader = nullptr;
		LowLevel::VertexDescription* vtxDesc = nullptr;

		LowLevel::Texture* blankTexture = {};

		/* Helper functions */

		void InitializeIndexBuffer();
	};
};
