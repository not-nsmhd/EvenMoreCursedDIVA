#include <array>
#include <vector>
#include "common/math_ext.h"
#include <glm/ext.hpp>
#include "SpriteRenderer.h"

namespace Starshine::GFX::Render2D
{
	using std::array;
	using std::vector;
	using namespace Common;

	constexpr size_t MaxSprites = 1024;
	constexpr size_t MaxVertices = MaxSprites * 4;
	constexpr size_t MaxIndices = MaxSprites * 6;

	struct SpriteVertexColors
	{
		Color TopLeft;
		Color TopRight;
		Color BottomLeft;
		Color BottomRight;
	};

	struct SpriteState
	{
		vec2 Position{};
		vec2 Origin{};
		vec2 Size{};
		SpriteVertexColors VertexColors{};

		float RotationCos = 0.0f;
		float RotationSin = 0.0f;

		RectangleF SourceRect_TexSpace{};
		bool FlipHorizontal = false;
		bool FlipVertical = false;
	};

	struct SpriteList
	{
		u32 FirstSpriteIndex = 0;
		u32 SpriteCount = 0;
		Texture* Texture = nullptr;
	};

	struct SpriteVertex
	{
		vec2 Position{};
		vec2 TexCoord{};
		Color Color{};
	};

	constexpr array<VertexAttrib, 3> SpriteVertexAttribs =
	{
		Detail::ConstructVertexAttrib(VertexAttribType::Position, 0, VertexAttribFormat::Float, 2, false, sizeof(SpriteVertex), offsetof(SpriteVertex, Position)),
		Detail::ConstructVertexAttrib(VertexAttribType::TexCoord, 0, VertexAttribFormat::Float, 2, false, sizeof(SpriteVertex), offsetof(SpriteVertex, TexCoord)),
		Detail::ConstructVertexAttrib(VertexAttribType::Color, 0, VertexAttribFormat::UnsignedByte, 4, true, sizeof(SpriteVertex), offsetof(SpriteVertex, Color))
	};

	struct BlendModeDescriptor
	{
		BlendFactor SourceColor{};
		BlendFactor DestinationColor{};
		BlendFactor SourceAlpha{};
		BlendFactor DestinationAlpha{};
		BlendOperation Operation{};
	};

	constexpr array<BlendModeDescriptor, EnumCount<BlendMode>()> BlendModes =
	{
		BlendModeDescriptor
		{ BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add },
		{ BlendFactor::SrcAlpha, BlendFactor::One, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add },
		{ BlendFactor::DestColor, BlendFactor::Zero, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add },
		{ BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add }
	};

	struct SpriteRenderer::Impl
	{
		Renderer* BaseRenderer = nullptr;

		struct
		{
			VertexBuffer* VertexBuffer = nullptr;
			IndexBuffer* IndexBuffer = nullptr;
			VertexDesc* VertexDesc = nullptr;
		} GraphicsResources;

		struct
		{
			Shader* DefaultShader = nullptr;
			Texture* DefaultTexture = nullptr;
		} DefaultSpriteResources;

		struct
		{
			mat4 TransformMatrix;
		} ShaderVariables;

		struct
		{
			ShaderVariableIndex TransformMatrix;
		} ShaderVariableBindings;

		vector<SpriteState> Sprites;
		vector<SpriteList> SpriteLists;
		vector<SpriteVertex> SpriteVertices;

		u32 PushedSprites = 0;
		u32 PushedSpriteLists = 0;

		SpriteState CurrentSprite{};
		SpriteList CurrentList{};

	public:
		Impl()
		{
			BaseRenderer = Renderer::GetInstance();

			Internal_CreateVertexBuffer();
			Internal_CreateIndexBuffer();
			Internal_CreateDefaultSpriteResources();

			SetBlendMode(BlendMode::Normal);
		}

		void Destroy()
		{
			BaseRenderer->DeleteResource(GraphicsResources.VertexBuffer);
			BaseRenderer->DeleteResource(GraphicsResources.IndexBuffer);
			BaseRenderer->DeleteResource(GraphicsResources.VertexDesc);
			BaseRenderer->DeleteResource(DefaultSpriteResources.DefaultShader);
			BaseRenderer->DeleteResource(DefaultSpriteResources.DefaultTexture);

			Sprites.clear();
			SpriteVertices.clear();

			BaseRenderer = nullptr;
		}

		void Internal_CreateVertexBuffer()
		{
			size_t vertexBufferSize = MaxVertices * sizeof(SpriteVertex);
			GraphicsResources.VertexBuffer = BaseRenderer->CreateVertexBuffer(vertexBufferSize, nullptr, true);

			GraphicsResources.VertexDesc = BaseRenderer->CreateVertexDesc(SpriteVertexAttribs.data(), SpriteVertexAttribs.size());
		}

		void Internal_CreateIndexBuffer()
		{
			array<u16, MaxIndices> indexData;

			// NOTE: Vertex order:
			//		 [0] - Top left,  [1] - Bottom right,
			//		 [2] - Top right, [3] - Bottom left
			// 
			//		 Index order:
			//		 [0] - Top left(0),     [1] - Top right(2),
			//		 [2] - Bottom right(1), [3] - Bottom left(3)

			u16 baseVertex = 0;
			for (size_t i = 0; i < indexData.size(); i += 6)
			{
				// NOTE: Indices are always arranged in clockwise order regardless of backend
				// (OpenGL is switchted to clockwise order on initialization, D3D9 uses clockwise order by default)
				indexData[i + 0] = baseVertex + 0;
				indexData[i + 1] = baseVertex + 2;
				indexData[i + 2] = baseVertex + 1;
				indexData[i + 3] = baseVertex + 1;
				indexData[i + 4] = baseVertex + 3;
				indexData[i + 5] = baseVertex + 0;

				baseVertex += 4;
			}

			size_t indexBufferSize = indexData.size() * sizeof(u16);
			GraphicsResources.IndexBuffer = BaseRenderer->CreateIndexBuffer(indexBufferSize, IndexFormat::Index16bit, indexData.data(), false);
		}
	
		void Internal_CreateDefaultSpriteResources()
		{
			DefaultSpriteResources.DefaultShader = BaseRenderer->LoadShaderFromXml("diva/shaders/SpriteDefault.xml");
			DefaultSpriteResources.DefaultTexture = BaseRenderer->CreateTexture(1, 1, TextureFormat::RGBA8, false, true);

			ShaderVariableBindings.TransformMatrix = DefaultSpriteResources.DefaultShader->GetVariableIndex("TransformMatrix");

			array<u8, 4> defaultTexData = { 0xFF, 0xFF, 0xFF, 0xFF };
			DefaultSpriteResources.DefaultTexture->SetData(0, 0, 1, 1, defaultTexData.data());
		}

		void ResetSprite()
		{
			CurrentSprite = {};

			CurrentSprite.RotationCos = SDL_cosf(0.0f);
			CurrentSprite.RotationSin = SDL_sinf(0.0f);

			CurrentSprite.VertexColors.TopLeft = { 255, 255, 255, 255 };
			CurrentSprite.VertexColors.TopRight = { 255, 255, 255, 255 };
			CurrentSprite.VertexColors.BottomLeft = { 255, 255, 255, 255 };
			CurrentSprite.VertexColors.BottomRight = { 255, 255, 255, 255 };

			CurrentSprite.SourceRect_TexSpace = { 0.0f, 0.0f, 1.0f, 1.0f };
		}

		void ResetList()
		{
			CurrentList = {};
		}

		void PushSprite(Texture* texture)
		{
			if (PushedSprites >= MaxSprites)
			{
				RenderSprites(nullptr);
			}

			PushedSprites++;
			Sprites.push_back(CurrentSprite);

			ResetSprite();

			Texture* listTex = (texture != nullptr) ? texture : DefaultSpriteResources.DefaultTexture;

			if (CurrentList.Texture != listTex)
			{
				if (PushedSpriteLists == 0)
				{
					CurrentList.Texture = listTex;

					CurrentList.SpriteCount++;
					PushedSpriteLists = 1;
				}
				else
				{
					SpriteLists.push_back(CurrentList);
					PushedSpriteLists++;

					CurrentList.Texture = listTex;
					CurrentList.FirstSpriteIndex = PushedSprites;
					CurrentList.SpriteCount = 1;
				}
			}
			else
			{
				CurrentList.SpriteCount++;
			}
		}

		void RenderSprites(Shader* shader)
		{
			if (PushedSprites == 0)
			{
				return;
			}

			if (SpriteLists.size() == 0)
			{
				SpriteLists.push_back(CurrentList);
			}

			size_t spriteVertexCount = static_cast<size_t>(PushedSprites) * 4;
			if (SpriteVertices.size() < spriteVertexCount)
			{
				SpriteVertices.insert(SpriteVertices.cbegin(), spriteVertexCount - SpriteVertices.size(), SpriteVertex());
			}

			size_t baseVertex = 0;
			for (auto& curSprite = Sprites.cbegin(); curSprite != Sprites.cbegin() + PushedSprites; curSprite++)
			{
				// Top-left
				SpriteVertex* vertex = &SpriteVertices[baseVertex + 0];
				vec2 basePos = {};

				vertex->Position = MathExtensions::RotateVector(basePos, curSprite->Origin, curSprite->RotationCos, curSprite->RotationSin) + curSprite->Position;
				vertex->TexCoord = { curSprite->SourceRect_TexSpace.X, curSprite->SourceRect_TexSpace.Y };
				vertex->Color = curSprite->VertexColors.TopLeft;

				// Bottom-right
				vertex = &SpriteVertices[baseVertex + 1];
				basePos = { curSprite->Size.x, curSprite->Size.y };

				vertex->Position = MathExtensions::RotateVector(basePos, curSprite->Origin, curSprite->RotationCos, curSprite->RotationSin) + curSprite->Position;
				vertex->TexCoord = { curSprite->SourceRect_TexSpace.Width, curSprite->SourceRect_TexSpace.Height };
				vertex->Color = curSprite->VertexColors.BottomRight;

				// Top-right
				vertex = &SpriteVertices[baseVertex + 2];
				basePos = { curSprite->Size.x, 0.0f };

				vertex->Position = MathExtensions::RotateVector(basePos, curSprite->Origin, curSprite->RotationCos, curSprite->RotationSin) + curSprite->Position;
				vertex->TexCoord = { curSprite->SourceRect_TexSpace.Width, curSprite->SourceRect_TexSpace.Y };
				vertex->Color = curSprite->VertexColors.TopRight;

				// Bottom-left
				vertex = &SpriteVertices[baseVertex + 3];
				basePos = { 0.0f, curSprite->Size.y };

				vertex->Position = MathExtensions::RotateVector(basePos, curSprite->Origin, curSprite->RotationCos, curSprite->RotationSin) + curSprite->Position;
				vertex->TexCoord = { curSprite->SourceRect_TexSpace.X, curSprite->SourceRect_TexSpace.Height };
				vertex->Color = curSprite->VertexColors.BottomLeft;

				baseVertex += 4;
			}

			GraphicsResources.VertexBuffer->SetData(SpriteVertices.data(), 0, static_cast<size_t>(PushedSprites) * sizeof(SpriteVertex) * 4);
			Shader* spriteShader = (shader != nullptr) ? shader : DefaultSpriteResources.DefaultShader;

			RectangleF viewportSize = BaseRenderer->GetViewportSize();
			ShaderVariables.TransformMatrix = glm::ortho(viewportSize.X, viewportSize.Width, viewportSize.Height, viewportSize.Y, 0.0f, 1.0f);
			spriteShader->SetVariableValue(ShaderVariableBindings.TransformMatrix, &ShaderVariables.TransformMatrix);

			BaseRenderer->SetVertexBuffer(GraphicsResources.VertexBuffer);
			BaseRenderer->SetVertexDesc(GraphicsResources.VertexDesc);
			BaseRenderer->SetIndexBuffer(GraphicsResources.IndexBuffer);
			BaseRenderer->SetShader(spriteShader);

			for (auto& list = SpriteLists.cbegin(); list != SpriteLists.cbegin() + PushedSpriteLists; list++)
			{
				BaseRenderer->SetTexture(list->Texture, 0);
				BaseRenderer->DrawIndexed(PrimitiveType::Triangles, list->FirstSpriteIndex * 6, list->SpriteCount * 6);
			}

			Sprites.clear();
			SpriteLists.clear();

			PushedSprites = 0;
			PushedSpriteLists = 0;

			ResetSprite();
			ResetList();
		}

		void SetBlendMode(BlendMode mode)
		{
			const BlendModeDescriptor& desc = BlendModes[static_cast<size_t>(mode)];
			BaseRenderer->SetBlendState(true, desc.SourceColor, desc.DestinationColor, desc.SourceAlpha, desc.DestinationAlpha);
			BaseRenderer->SetBlendOperation(desc.Operation);
		}
	};

	SpriteRenderer::SpriteRenderer() : impl(new Impl())
	{
	}

	SpriteRenderer::~SpriteRenderer()
	{
	}

	void SpriteRenderer::Destroy()
	{
		impl->Destroy();
	}

	void SpriteRenderer::ResetSprite()
	{
		impl->ResetSprite();
	}

	void SpriteRenderer::SetSpritePosition(vec2& position)
	{
		impl->CurrentSprite.Position = position;
	}

	void SpriteRenderer::SetSpriteScale(vec2& absScale)
	{
		impl->CurrentSprite.Size = absScale;
	}

	void SpriteRenderer::SetSpriteOrigin(vec2& origin)
	{
		impl->CurrentSprite.Origin = origin;
	}

	void SpriteRenderer::SetSpriteRotation(float radians)
	{
		impl->CurrentSprite.RotationCos = SDL_cosf(radians);
		impl->CurrentSprite.RotationSin = SDL_sinf(radians);
	}

	void SpriteRenderer::SetSpriteSource(RectangleF& source)
	{
		impl->CurrentSprite.SourceRect_TexSpace = source;
	}

	void SpriteRenderer::SetSpriteSource(const Texture* texture, RectangleF& absSource)
	{
		u32 width = texture->GetWidth();
		u32 height = texture->GetHeight();

		float w = (width > 0) ? static_cast<float>(width) : 1.0f;
		float h = (height > 0) ? static_cast<float>(height) : 1.0f;

		impl->CurrentSprite.SourceRect_TexSpace.X = absSource.X / w;
		impl->CurrentSprite.SourceRect_TexSpace.Width = (absSource.X + absSource.Width) / w;
		impl->CurrentSprite.SourceRect_TexSpace.Y = absSource.Y / h;
		impl->CurrentSprite.SourceRect_TexSpace.Height = (absSource.Y + absSource.Height) / h;
	}
	
	void SpriteRenderer::SetSpriteFlip(bool flipHorizontal, bool flipVertical)
	{
		impl->CurrentSprite.FlipHorizontal = flipHorizontal;
		impl->CurrentSprite.FlipVertical = flipVertical;
	}

	void SpriteRenderer::SetSpriteColor(Color color)
	{
		impl->CurrentSprite.VertexColors.TopLeft = color;
		impl->CurrentSprite.VertexColors.TopRight = color;
		impl->CurrentSprite.VertexColors.BottomLeft = color;
		impl->CurrentSprite.VertexColors.BottomRight = color;
	}
	
	void SpriteRenderer::SetSpriteColors(const Color colors[4])
	{
		impl->CurrentSprite.VertexColors.TopLeft = colors[0];
		impl->CurrentSprite.VertexColors.TopRight = colors[1];
		impl->CurrentSprite.VertexColors.BottomLeft = colors[2];
		impl->CurrentSprite.VertexColors.BottomRight = colors[3];
	}

	void SpriteRenderer::SetSpriteColors(Color& topLeft, Color& topRight, Color& bottomLeft, Color& bottomRight)
	{
		impl->CurrentSprite.VertexColors.TopLeft = topLeft;
		impl->CurrentSprite.VertexColors.TopRight = topRight;
		impl->CurrentSprite.VertexColors.BottomLeft = bottomLeft;
		impl->CurrentSprite.VertexColors.BottomRight = bottomRight;
	}

	void SpriteRenderer::SetBlendMode(BlendMode mode)
	{
		impl->SetBlendMode(mode);
	}

	void SpriteRenderer::PushSprite(Texture* texture)
	{
		impl->PushSprite(texture);
	}

	void SpriteRenderer::RenderSprites(Shader* shader)
	{
		impl->RenderSprites(shader);
	}
};
