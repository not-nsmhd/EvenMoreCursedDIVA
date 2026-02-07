#include "SpriteRenderer.h"
#include "Rendering/Utilities.h"
#include <array>
#include <vector>
#include <Common/MathExt.h>
#include <glm/ext.hpp>

namespace Starshine::Rendering::Render2D
{
	using std::array;
	using std::vector;
	using namespace GFX;

	constexpr size_t MaxSprites = 1024;
	constexpr size_t MaxLists = 1024;
	constexpr size_t MaxVertices = MaxSprites * 4;
	constexpr size_t MaxIndices = MaxSprites * 6;

	constexpr size_t MaxShapeVertices = 1024;

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

	struct DrawCommand
	{
		u32 FirstSpriteIndex = 0;
		u32 SpriteCount = 0;

		u32 ShapeFirstVertex = 0;
		u32 ShapeVertexCount = 0;

		PrimitiveType PrimitiveType = PrimitiveType::Triangles;
		Texture* Texture = nullptr;
	};

	constexpr array<VertexAttrib, 3> SpriteVertexAttribs
	{
		VertexAttrib { VertexAttribType::Position, 0, VertexAttribFormat::Float2, sizeof(SpriteVertex), offsetof(SpriteVertex, Position) },
		VertexAttrib { VertexAttribType::TexCoord, 0, VertexAttribFormat::Float2, sizeof(SpriteVertex), offsetof(SpriteVertex, TexCoord) },
		VertexAttrib { VertexAttribType::Color, 0, VertexAttribFormat::UnsignedByte4Norm, sizeof(SpriteVertex), offsetof(SpriteVertex, Color) }
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
		Device* GFXDevice = nullptr;

		SpriteSheetRenderer SpriteSheetRenderer;
		FontRenderer FontRenderer;

		struct
		{
			std::unique_ptr<VertexBuffer> SpriteVertexBuffer = nullptr;
			std::unique_ptr<IndexBuffer> SpriteIndexBuffer = nullptr;
			std::unique_ptr<VertexDesc> VertexDesc = nullptr;

			std::unique_ptr<VertexBuffer> ShapeVertexBuffer = nullptr;
		} GraphicsResources;

		struct
		{
			std::unique_ptr<Shader> DefaultShader = nullptr;
			std::unique_ptr<Texture> DefaultTexture = nullptr;
		} DefaultSpriteResources;

		struct
		{
			mat4 TransformMatrix;
		} ShaderVariables;

		vector<SpriteState> Sprites;
		vector<DrawCommand> DrawCommands;
		vector<SpriteVertex> SpriteVertices;

		vector<SpriteVertex> ShapeVertices;

		u32 PushedSprites = 0;
		u32 PushedShapeVertices = 0;
		u32 PushedDrawCommands = 0;

		SpriteState CurrentSprite{};
		DrawCommand CurrentList{};

	public:
		Impl(SpriteRenderer& parent) : SpriteSheetRenderer(parent), FontRenderer(parent)
		{
			GFXDevice = Rendering::GetDevice();

			Internal_CreateVertexBuffer();
			Internal_CreateIndexBuffer();
			Internal_CreateDefaultSpriteResources();

			SetBlendMode(BlendMode::Normal);
		}

		void Destroy()
		{
			GraphicsResources.SpriteVertexBuffer = nullptr;
			GraphicsResources.SpriteIndexBuffer = nullptr;
			GraphicsResources.ShapeVertexBuffer = nullptr;
			GraphicsResources.VertexDesc = nullptr;
			DefaultSpriteResources.DefaultShader = nullptr;
			DefaultSpriteResources.DefaultTexture = nullptr;

			Sprites.clear();
			SpriteVertices.clear();
			ShapeVertices.clear();
		}

		void Internal_CreateVertexBuffer()
		{
			size_t vertexBufferSize = MaxVertices * sizeof(SpriteVertex);
			GraphicsResources.SpriteVertexBuffer = GFXDevice->CreateVertexBuffer(vertexBufferSize, nullptr, true);

			vertexBufferSize = MaxShapeVertices * sizeof(SpriteVertex);
			GraphicsResources.ShapeVertexBuffer = GFXDevice->CreateVertexBuffer(vertexBufferSize, nullptr, true);

			GraphicsResources.VertexDesc = GFXDevice->CreateVertexDesc(SpriteVertexAttribs.data(), SpriteVertexAttribs.size());
		}

		void Internal_CreateIndexBuffer()
		{
			array<u16, MaxIndices> indexData{};

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
			GraphicsResources.SpriteIndexBuffer = GFXDevice->CreateIndexBuffer(indexBufferSize, IndexFormat::Index16bit, indexData.data(), false);
		}
	
		void Internal_CreateDefaultSpriteResources()
		{
			DefaultSpriteResources.DefaultShader = Rendering::Utilities::LoadShader("diva/shaders/d3d9/VS_SpriteDefault.cso", "diva/shaders/d3d9/FS_SpriteDefault.cso");
			DefaultSpriteResources.DefaultTexture = GFXDevice->CreateTexture(1, 1, TextureFormat::RGBA8, false, false);

			array<u8, 4> defaultTexData = { 0xFF, 0xFF, 0xFF, 0xFF };
			DefaultSpriteResources.DefaultTexture->SetData(defaultTexData.data(), 0, 0, 1, 1);
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

			Texture* listTex = (texture != nullptr) ? texture : DefaultSpriteResources.DefaultTexture.get();

			if (CurrentList.Texture != listTex || CurrentList.ShapeVertexCount != 0)
			{
				if (PushedDrawCommands == 0)
				{
					CurrentList.Texture = listTex;
					CurrentList.PrimitiveType = PrimitiveType::Triangles;

					CurrentList.SpriteCount++;
					PushedDrawCommands = 1;
				}
				else
				{
					DrawCommands.push_back(CurrentList);
					PushedDrawCommands++;

					CurrentList.Texture = listTex;
					CurrentList.PrimitiveType = PrimitiveType::Triangles;

					CurrentList.FirstSpriteIndex = PushedSprites - 1;
					CurrentList.SpriteCount = 1;
				}

				CurrentList.ShapeFirstVertex = 0;
				CurrentList.ShapeVertexCount = 0;
			}
			else
			{
				CurrentList.SpriteCount++;
			}
		}

		void RenderSprites(Shader* shader)
		{
			if (PushedSprites == 0 && PushedShapeVertices == 0)
			{
				return;
			}

			DrawCommands.push_back(CurrentList);

			size_t spriteVertexCount = static_cast<size_t>(PushedSprites) * 4;
			if (SpriteVertices.size() < spriteVertexCount)
			{
				SpriteVertices.insert(SpriteVertices.cbegin(), spriteVertexCount - SpriteVertices.size(), SpriteVertex());
			}

			size_t baseVertex = 0;
			for (auto curSprite = Sprites.cbegin(); curSprite != Sprites.cbegin() + PushedSprites; curSprite++)
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

			if (!ShapeVertices.empty())
			{
				GraphicsResources.ShapeVertexBuffer->SetData(ShapeVertices.data(), 0, ShapeVertices.size() * sizeof(SpriteVertex));
			}

			if (PushedSprites > 0)
			{
				GraphicsResources.SpriteVertexBuffer->SetData(SpriteVertices.data(), 0, static_cast<size_t>(PushedSprites) * sizeof(SpriteVertex) * 4);
			}

			Shader* spriteShader = (shader != nullptr) ? shader : DefaultSpriteResources.DefaultShader.get();

			RectangleF viewportSize = GFXDevice->GetViewportSize();
			ShaderVariables.TransformMatrix = glm::orthoRH_ZO(viewportSize.X, viewportSize.Width, viewportSize.Height, viewportSize.Y, 0.0f, 1.0f);
			spriteShader->SetVertexShaderMatrix(0, ShaderVariables.TransformMatrix);

			GFXDevice->SetIndexBuffer(GraphicsResources.SpriteIndexBuffer.get());
			GFXDevice->SetShader(spriteShader);

			bool switchBackToSpriteBuffer = true;

			for (auto list = DrawCommands.cbegin(); list != DrawCommands.cend(); list++)
			{
				GFXDevice->SetTexture(list->Texture, 0);
				if (list->ShapeVertexCount == 0)
				{
					if (switchBackToSpriteBuffer)
					{
						GFXDevice->SetVertexBuffer(GraphicsResources.SpriteVertexBuffer.get(), GraphicsResources.VertexDesc.get());
						switchBackToSpriteBuffer = false;
					}
					GFXDevice->DrawIndexed(PrimitiveType::Triangles, list->FirstSpriteIndex * 6, PushedSprites * 4, list->SpriteCount * 6);
				}
				else
				{
					GFXDevice->SetVertexBuffer(GraphicsResources.ShapeVertexBuffer.get(), GraphicsResources.VertexDesc.get());
					GFXDevice->DrawArrays(list->PrimitiveType, list->ShapeFirstVertex, list->ShapeVertexCount);
					switchBackToSpriteBuffer = true;
				}
			}

			Sprites.clear();
			ShapeVertices.clear();
			DrawCommands.clear();

			PushedSprites = 0;
			PushedDrawCommands = 0;

			ResetSprite();
			ResetList();
		}

		void PushShape(const SpriteVertex* vertices, size_t vertexCount, PrimitiveType primType, Texture* texture)
		{
			if (PushedDrawCommands + 1 >= MaxLists || ShapeVertices.size() + vertexCount >= MaxShapeVertices) { RenderSprites(nullptr); }

			Texture* listTex = (texture != nullptr) ? texture : DefaultSpriteResources.DefaultTexture.get();

			if (CurrentList.SpriteCount != 0 || CurrentList.PrimitiveType != primType || CurrentList.Texture != listTex)
			{
				if (PushedDrawCommands == 0)
				{
					PushedDrawCommands++;
				}
				else
				{
					DrawCommands.push_back(CurrentList);
					PushedDrawCommands++;

					CurrentList.FirstSpriteIndex = 0;
					CurrentList.SpriteCount = 0;
				}
			}

			CurrentList.Texture = listTex;
			CurrentList.PrimitiveType = primType;

			CurrentList.ShapeFirstVertex = ShapeVertices.size();
			CurrentList.ShapeVertexCount = vertexCount;

			ShapeVertices.reserve(ShapeVertices.size() + vertexCount);

			for (size_t i = 0; i < vertexCount; i++)
			{
				const SpriteVertex* srcVertex = &vertices[i];
				SpriteVertex& newVertex = ShapeVertices.emplace_back();

				newVertex.Position = srcVertex->Position;
				newVertex.TexCoord = srcVertex->TexCoord;
				newVertex.Color = srcVertex->Color;
			}

			PushedShapeVertices += vertexCount;
		}

		void SetBlendMode(BlendMode mode)
		{
			const BlendModeDescriptor& desc = BlendModes[static_cast<size_t>(mode)];
			GFXDevice->SetBlendState(true, desc.SourceColor, desc.DestinationColor, desc.SourceAlpha, desc.DestinationAlpha);
			GFXDevice->SetBlendOperation(desc.Operation);
		}
	};

	SpriteRenderer::SpriteRenderer() : impl(new Impl(*this))
	{
	}

	SpriteRenderer::~SpriteRenderer()
	{
	}

	void SpriteRenderer::Destroy()
	{
		impl->Destroy();
	}

	Device* SpriteRenderer::GetRenderingDevice()
	{
		return impl->GFXDevice;
	}

	void SpriteRenderer::ResetSprite()
	{
		impl->ResetSprite();
	}

	void SpriteRenderer::SetSpritePosition(const vec2& position)
	{
		impl->CurrentSprite.Position = position;
	}

	void SpriteRenderer::SetSpriteSize(const vec2& size)
	{
		impl->CurrentSprite.Size = size;
	}

	void SpriteRenderer::SetSpriteOrigin(const vec2& origin)
	{
		impl->CurrentSprite.Origin = origin;
	}

	void SpriteRenderer::SetSpriteRotation(float radians)
	{
		impl->CurrentSprite.RotationCos = SDL_cosf(radians);
		impl->CurrentSprite.RotationSin = SDL_sinf(radians);
	}

	void SpriteRenderer::SetSpriteSource(const RectangleF& texSpaceSource)
	{
		impl->CurrentSprite.SourceRect_TexSpace = texSpaceSource;
	}

	void SpriteRenderer::SetSpriteSource(const Texture* texture, const RectangleF& absSource)
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

	void SpriteRenderer::SetSpriteColor(const Color& color)
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

	void SpriteRenderer::SetSpriteColors(const Color& topLeft, const Color& topRight, const Color& bottomLeft, const Color& bottomRight)
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

	void SpriteRenderer::PushShape(const SpriteVertex* vertices, size_t vertexCount, PrimitiveType primType, Texture* texture)
	{
		impl->PushShape(vertices, vertexCount, primType, texture);
	}

	void SpriteRenderer::PushLine(const vec2& position, float angle, float length, const Color& color, float thickness)
	{
		SetSpritePosition(position);
		SetSpriteSize({ length, thickness });
		SetSpriteRotation(angle);
		SetSpriteColor(color);
		PushSprite(nullptr);
	}

	void SpriteRenderer::PushOutlineRect(const vec2& position, const vec2& size, const vec2& origin, const Color& color, float thickness)
	{
		SetSpritePosition({ position.x - origin.x, position.y - origin.y });
		SetSpriteSize({ size.x, thickness });
		SetSpriteColor(color);
		PushSprite(nullptr);

		SetSpritePosition({ position.x - origin.x, position.y - origin.y });
		SetSpriteSize({ thickness, size.y });
		SetSpriteColor(color);
		PushSprite(nullptr);

		SetSpritePosition({ position.x + size.x - thickness - origin.x, position.y - origin.y });
		SetSpriteSize({ thickness, size.y });
		SetSpriteColor(color);
		PushSprite(nullptr);

		SetSpritePosition({ position.x - origin.x, position.y + size.y - thickness - origin.y });
		SetSpriteSize({ size.x, thickness });
		SetSpriteColor(color);
		PushSprite(nullptr);
	}

	SpriteSheetRenderer& SpriteRenderer::SpriteSheet()
	{
		return impl->SpriteSheetRenderer;
	}

	FontRenderer& SpriteRenderer::Font()
	{
		return impl->FontRenderer;
	}
};
