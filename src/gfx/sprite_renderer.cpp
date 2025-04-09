#include <SDL2/SDL.h>
#include <glm/ext.hpp>
#include "../common/math_ext.h"
#include "helpers/tex_helpers.h"
#include "helpers/shader_helpers.h"
#include "sprite_renderer.h"

using namespace GFX::Helpers;
using namespace GFX::LowLevel;

namespace GFX
{
#pragma region Internal Helper Functions
	void SpriteRenderer::InitializeIndexBuffer()
	{
		u16* indexData = new u16[MAX_INDICES];

		for (size_t i = 0, v = 0; i < MAX_INDICES; i += 6, v += 4)
		{
			indexData[i + 0] = (u16)(v + 0);
			indexData[i + 1] = (u16)(v + 3);
			indexData[i + 2] = (u16)(v + 1);
			indexData[i + 3] = (u16)(v + 1);
			indexData[i + 4] = (u16)(v + 2);
			indexData[i + 5] = (u16)(v + 0);
		}

		indexBuffer = gfxBackend->CreateIndexBuffer
		(
			BufferUsage::BUFFER_USAGE_STATIC, 
			IndexFormat::INDEX_16BIT,
			indexData, 
			MAX_INDICES * sizeof(u16)
		);

		delete[] indexData;
	}
#pragma endregion

#pragma region Implementation
	SpriteRenderer::SpriteRenderer()
	{
	}

	SpriteRenderer::~SpriteRenderer()
	{
		if (initialized)
		{
			Destroy();
		}
	}
	
	void SpriteRenderer::Initialize(Backend* backend)
	{
		if (initialized)
		{
			return;
		}

		gfxBackend = backend;
		uniforms.projMatrix = glm::orthoLH_ZO(0.0f, 1280.0f, 720.0f, 0.0f, 0.0f, 1.0f);

		defaultShader = LoadShaderFromDescriptor(gfxBackend, "shaders/SpriteDefault.xml");

		VertexAttribute vtxAttribs[] =
		{
			{ VertexAttributeType::POSITION, 0, VertexAttributeFormat::VERT_FORMAT_FLOAT2, false, false, offsetof(SpriteVertex, pos) },
			{ VertexAttributeType::TEXCOORD, 0, VertexAttributeFormat::VERT_FORMAT_FLOAT2, false, false, offsetof(SpriteVertex, texCoord) },
			{ VertexAttributeType::COLOR, 0, VertexAttributeFormat::VERT_FORMAT_BYTE4, true, true, offsetof(SpriteVertex, color) }
		};

		vertexBuffer = gfxBackend->CreateVertexBuffer(BufferUsage::BUFFER_USAGE_DYNAMIC, nullptr, MAX_VERTICES * sizeof(SpriteVertex));
		InitializeIndexBuffer();

		vtxDesc = gfxBackend->CreateVertexDescription(vtxAttribs, 3, sizeof(SpriteVertex), defaultShader);

		vertexData = new SpriteVertex[MAX_VERTICES];

		u32 pixel = 0xFFFFFFFF;
		blankTexture = gfxBackend->CreateTexture(1, 1, TextureFormat::TEXFORMAT_RGBA8, 0);
		gfxBackend->SetTextureData(blankTexture, &pixel);

		sprites.push_back({});
		batches.push_back({});

		currentSprite = sprites.begin();
		currentBatch = batches.begin();

		ResetSprite();

		initialized = true;
	}

	void SpriteRenderer::Destroy()
	{
		if (!initialized)
		{
			return;
		}

		gfxBackend->DestroyTexture(blankTexture);
		gfxBackend->DestroyVertexDescription(vtxDesc);
		gfxBackend->DestroyShader(defaultShader);
		gfxBackend->DestroyBuffer(vertexBuffer);
		gfxBackend->DestroyBuffer(indexBuffer);
		delete[] vertexData;

		batches.clear();
		sprites.clear();

		initialized = false;
	}

	void SpriteRenderer::ResetSprite()
	{
		currentSprite->position = { 0.0f, 0.0f };
		currentSprite->origin = { 0.0f, 0.0f };
		currentSprite->size = { 0.0f, 0.0f };
		SDL_memset(currentSprite->colors, 0xFF, sizeof(struct Color) * 4);

		currentSprite->rotCos = 1.0f;
		currentSprite->rotSin = 0.0f;

		currentSprite->srcRect = { 0.0f, 0.0f, 1.0f, 1.0f };
		currentSprite->flipFlags = 0;
	}

	void SpriteRenderer::SetSpritePosition(vec2 position)
	{
		currentSprite->position = position;
	}

	void SpriteRenderer::SetSpriteScale(vec2 absScale)
	{
		currentSprite->size = absScale;
	}

	void SpriteRenderer::SetSpriteScale(const Texture* texture, vec2 scale)
	{
		currentSprite->size = { texture->GetWidth() * scale.x, texture->GetHeight() * scale.y };
	}

	void SpriteRenderer::SetSpriteOrigin(vec2 origin)
	{
		currentSprite->origin = origin;
	}

	void SpriteRenderer::SetSpriteRotation(float radians)
	{
		currentSprite->rotCos = SDL_cosf(radians);
		currentSprite->rotSin = SDL_sinf(radians);
	}

	void SpriteRenderer::SetSpriteSource(RectangleF source)
	{
		currentSprite->srcRect = source;
	}

	void SpriteRenderer::SetSpriteSource(const Texture* texture, RectangleF absSource)
	{
		int width = texture->GetWidth();
		int height = texture->GetHeight();

		float w = (width > 0) ? static_cast<float>(width) : 1.0f;
		float h = (height > 0) ? static_cast<float>(height) : 1.0f;

		currentSprite->srcRect.x = absSource.x / w;
		currentSprite->srcRect.width = (absSource.x + absSource.width) / w;
		currentSprite->srcRect.y = absSource.y / h;
		currentSprite->srcRect.height = (absSource.y + absSource.height) / h;
	}
	
	void SpriteRenderer::SetSpriteFlip(u32 flipFlags)
	{
		if (flipFlags > 2)
		{
			return;
		}

		currentSprite->flipFlags = flipFlags;
	}

	void SpriteRenderer::SetSpriteColor(struct Color color)
	{
		currentSprite->colors[0] = color;
		currentSprite->colors[1] = color;
		currentSprite->colors[2] = color;
		currentSprite->colors[3] = color;
	}
	
	void SpriteRenderer::SetSpriteColors(struct Color colors[4])
	{
		currentSprite->colors[0] = colors[0];
		currentSprite->colors[1] = colors[1];
		currentSprite->colors[2] = colors[2];
		currentSprite->colors[3] = colors[3];
	}

	void SpriteRenderer::PushSprite(Texture* texture)
	{
		if (spriteCount >= MAX_SPRITES)
		{
			RenderSprites(nullptr);
		}

		spriteCount++;

		if (sprites.size() < spriteCount + 1)
		{
			sprites.push_back({});
		}

		currentSprite = sprites.begin() + spriteCount;

		ResetSprite();

		Texture* sprTexture = (texture == nullptr) ? blankTexture : texture;

		if (currentBatch->texture != sprTexture)
		{
			if (batchCount == 0)
			{
				currentBatch->texture = sprTexture;

				currentBatch->firstSprite = sprites.begin();
				currentBatch->spriteCount++;
				batchCount = 1;
			}
			else
			{
				if (batches.size() < batchCount + 1)
				{
					batches.push_back({});
				}

				currentBatch = batches.begin() + batchCount;
				batchCount++;

				currentBatch->texture = sprTexture;
				currentBatch->firstSprite = sprites.begin() + spriteCount;
				currentBatch->spriteCount++;
			}
		}
		else
		{
			currentBatch->spriteCount++;
		}
	}

	void SpriteRenderer::RenderSprites(Shader* shader)
	{
		if (batchCount == 0)
		{
			return;
		}

		using namespace Common::MathExtensions;

		size_t vIndex = 0;
		for (vector<SpriteState>::iterator sprite = sprites.begin(); sprite < sprites.begin() + spriteCount; sprite++)
		{
			vec2 sprPos = sprite->position;
			vec2 sprSize = sprite->size;

			bool flipX = (sprite->flipFlags & SpriteFlipMode::FLIP_X) == SpriteFlipMode::FLIP_X;
			bool flipY = (sprite->flipFlags & SpriteFlipMode::FLIP_Y) == SpriteFlipMode::FLIP_Y;

			/* Top-left */
			vec2 vtxPos = { 0.0f, 0.0f };

			vertexData[vIndex].pos = RotateVector(vtxPos, sprite->origin, sprite->rotCos, sprite->rotSin) + sprPos;
			vertexData[vIndex].color = u8vec4(sprite->colors[0].R, sprite->colors[0].G, sprite->colors[0].B, sprite->colors[0].A);
			vertexData[vIndex].texCoord = { !flipX ? sprite->srcRect.x : sprite->srcRect.width, !flipY ? sprite->srcRect.y : sprite->srcRect.height };
			vIndex++;

			/* Bottom-right */
			vtxPos.x = sprSize.x;
			vtxPos.y = sprSize.y;

			vertexData[vIndex].pos = RotateVector(vtxPos, sprite->origin, sprite->rotCos, sprite->rotSin) + sprPos;
			vertexData[vIndex].color = u8vec4(sprite->colors[3].R, sprite->colors[3].G, sprite->colors[3].B, sprite->colors[3].A);
			vertexData[vIndex].texCoord = { !flipX ? sprite->srcRect.width : sprite->srcRect.x, !flipY ? sprite->srcRect.height : sprite->srcRect.y };			
			vIndex++;

			/* Bottom-left */
			vtxPos.x = 0.0f;
			vtxPos.y = sprSize.y;

			vertexData[vIndex].pos = RotateVector(vtxPos, sprite->origin, sprite->rotCos, sprite->rotSin) + sprPos;
			vertexData[vIndex].color = u8vec4(sprite->colors[2].R, sprite->colors[2].G, sprite->colors[2].B, sprite->colors[2].A);
			vertexData[vIndex].texCoord = { !flipX ? sprite->srcRect.x : sprite->srcRect.width, !flipY ? sprite->srcRect.height : sprite->srcRect.y };
			vIndex++;

			/* Top-right */
			vtxPos.x = sprSize.x;
			vtxPos.y = 0.0f;

			vertexData[vIndex].pos = RotateVector(vtxPos, sprite->origin, sprite->rotCos, sprite->rotSin) + sprPos;
			vertexData[vIndex].color = u8vec4(sprite->colors[1].R, sprite->colors[1].G, sprite->colors[1].B, sprite->colors[1].A);
			vertexData[vIndex].texCoord = { !flipX ? sprite->srcRect.width : sprite->srcRect.x, !flipY ? sprite->srcRect.y : sprite->srcRect.height };
			vIndex++;
		}

		gfxBackend->SetBufferData(vertexBuffer, vertexData, 0, spriteCount * 4 * sizeof(SpriteVertex));

		Shader* sprShader = (shader == nullptr) ? defaultShader : shader;

		gfxBackend->BindShader(sprShader);
		gfxBackend->SetVertexDescription(vtxDesc);
		gfxBackend->BindVertexBuffer(vertexBuffer);
		gfxBackend->BindIndexBuffer(indexBuffer);

		float x;
		float y;
		float width;
		float height;
		gfxBackend->GetViewportSize(&x, &y, &width, &height);
		uniforms.projMatrix = glm::orthoLH_ZO(0.0f, width, height, 0.0f, 0.0f, 1.0f);
		gfxBackend->SetShaderMatrix(0, &uniforms.projMatrix);

		size_t spriteOffset = 0;
		for (vector<SpriteBatch>::iterator batch = batches.begin(); batch != batches.begin() + batchCount; batch++)
		{
			gfxBackend->BindTexture(batch->texture, 0);

			gfxBackend->DrawIndexed(PrimitiveType::PRIMITIVE_TRIANGLES, batch->spriteCount * 4, spriteOffset * 6, batch->spriteCount * 6);
			spriteOffset += batch->spriteCount;

			batch->texture = nullptr;
			batch->spriteCount = 0;
		}

		currentSprite = sprites.begin();
		currentBatch = batches.begin();
		spriteCount = 0;
		batchCount = 0;

		ResetSprite();
	}
#pragma endregion
};
