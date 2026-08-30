#pragma once
#include "Common/Rect.h"
#include "Common/Color.h"
#include "Rendering/Device.h"
#include "SpriteSheetRenderer.h"
#include "FontRenderer.h"
#include "AnimationSetRenderer.h"

namespace Starshine::Rendering::Render2D
{
	struct SpriteVertex
	{
		vec2 Position{};
		vec2 TexCoord{};
		Color Color{};
	};

	enum class CommandType : i32
	{
		NotSet,

		DrawSprites,
		DrawShape,

		SetTexture,
		SetShader,
		SetBlendMode,

		SetTransformMatrix,

		Count
	};

	constexpr std::array<const char*, EnumCount<CommandType>()> CommandTypeNames
	{
		"NotSet",
		"DrawSprites",
		"DrawShape",
		"SetTexture",
		"SetShader",
		"SetBlendMode",
		"SetTransformMatrix"
	};

	constexpr size_t MaxFSUniformDataSize = 64;

	union DrawCommand
	{
		CommandType Type{};

		struct
		{
			CommandType Type{};

			u32 FirstSpriteIndex{};
			u32 SpriteCount{};
		} DrawSprites;

		struct
		{
			CommandType Type{};

			u32 FirstVertexIndex{};
			u32 VertexCount{};
			PrimitiveType PrimitiveType{};
		} DrawShape;

		struct
		{
			CommandType Type{};
			Graphics::Texture* Texture{};
		} SetTexture;

		struct
		{
			CommandType Type;
			const Shader* Shader{};
			u8 FSUniformData[MaxFSUniformDataSize]{};
		} SetShader;

		struct
		{
			CommandType Type{};
			Graphics::BlendMode Mode{};
		} SetBlendMode;

		struct
		{
			CommandType Type{};
			glm::mat4 Matrix{};
		} SetTransformMatrix;
	};

	class SpriteRenderer
	{
	public:
		SpriteRenderer();
		~SpriteRenderer();

	public:
		Device* GetRenderingDevice();

	public:
		void ResetSprite();
		void SetSpritePosition(const vec2& position);
		void SetSpriteSize(const vec2& absScale);
		void SetSpriteOrigin(const vec2& origin);
		void SetSpriteRotation(float radians);

		// NOTE: Source must be specified in texture space
		// (texture's point at its width and height is represented as a (1.0, 1.0) coordinate)
		void SetSpriteSource(RectangleF texSpaceSource);
		void SetSpriteSource(const Graphics::Texture* texture, RectangleF absSource);

		void SetSpriteColor(const Color& color);

		void SetBlendMode(Graphics::BlendMode mode);
		void SetShader(const Shader* shader, const void* fsData, size_t fsDataSize);
		void SetTransformMatrix(mat4 matrix);

		void PushSprite(Graphics::Texture* texture);
		void DrawShape(const SpriteVertex* vertices, size_t vertexCount, PrimitiveType primType, Graphics::Texture* texture);

		void Present();

	public:
		void PushLine(const vec2& position, float angle, float length, const Color& color, float thickness = 1.0f);
		void PushOutlineRect(const vec2& position, const vec2& size, const vec2& origin, const Color& color, float thickness = 1.0f);

	public:
		SpriteSheetRenderer& SpriteSheet();
		FontRenderer& Font();
		AnimationSetRenderer& AnimationSet();

	public:
		size_t GetDrawCommands(DrawCommand** commands);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl{};
	};
};
