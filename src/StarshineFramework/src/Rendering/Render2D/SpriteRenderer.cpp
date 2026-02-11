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

	constexpr size_t MaxSprites = 4096;
	constexpr size_t MaxLists = 2048;
	constexpr size_t MaxVertices = MaxSprites * 4;
	constexpr size_t MaxIndices = MaxSprites * 6;

	constexpr size_t MaxShapeVertices = 2048;

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

	struct BlendModeState
	{
		BlendStateDesc Desc{};
		std::unique_ptr<BlendState> StateObject{ nullptr };
	};

	array<BlendModeState, EnumCount<BlendMode>()> BlendModes
	{
		BlendModeState
		{ { BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add } },
		{ { BlendFactor::SrcAlpha, BlendFactor::One, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add } },
		{ { BlendFactor::DestColor, BlendFactor::Zero, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add } },
		{ { BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add } }
	};

	struct SpriteRenderer::Impl
	{
		Device* GFXDevice = nullptr;

		SpriteSheetRenderer SpriteSheetRenderer;
		FontRenderer FontRenderer;

		struct ShaderUniformsBufferData
		{
			mat4 TransformMatrix;
		} ShaderUniforms;

		struct
		{
			std::unique_ptr<VertexBuffer> SpriteVertexBuffer = nullptr;
			std::unique_ptr<IndexBuffer> SpriteIndexBuffer = nullptr;
			std::unique_ptr<VertexDesc> VertexDesc = nullptr;

			std::unique_ptr<VertexBuffer> ShapeVertexBuffer = nullptr;

			std::unique_ptr<UniformBuffer> ShaderUniformBuffer = nullptr;
		} GraphicsResources;

		struct
		{
			std::unique_ptr<Shader> DefaultShader = nullptr;
			std::unique_ptr<Texture> DefaultTexture = nullptr;
		} DefaultSpriteResources;

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

			Internal_CreateDefaultSpriteResources();
			Internal_CreateShaderUniformBuffer();

			Internal_CreateVertexBuffer();
			Internal_CreateIndexBuffer();
			Internal_CreateBlendStates();

			SetBlendMode(BlendMode::Normal);
		}

		~Impl()
		{
			Destroy();
		}

		void Destroy()
		{
#if 0
			GraphicsResources.SpriteVertexBuffer = nullptr;
			GraphicsResources.SpriteIndexBuffer = nullptr;
			GraphicsResources.ShapeVertexBuffer = nullptr;
			GraphicsResources.VertexDesc = nullptr;
			DefaultSpriteResources.DefaultShader = nullptr;
			DefaultSpriteResources.DefaultTexture = nullptr;
#endif

			Sprites.clear();
			SpriteVertices.clear();
			ShapeVertices.clear();
		}

		void Internal_CreateVertexBuffer()
		{
			size_t vertexBufferSize = MaxVertices * sizeof(SpriteVertex);
			GraphicsResources.SpriteVertexBuffer = GFXDevice->CreateVertexBuffer(vertexBufferSize, nullptr, true);
			GraphicsResources.SpriteVertexBuffer->SetDebugName("[Starshine] SpriteRenderer::SpriteVertexBuffer");

			vertexBufferSize = MaxShapeVertices * sizeof(SpriteVertex);
			GraphicsResources.ShapeVertexBuffer = GFXDevice->CreateVertexBuffer(vertexBufferSize, nullptr, true);
			GraphicsResources.ShapeVertexBuffer->SetDebugName("[Starshine] SpriteRenderer::ShapeVertexBuffer");

			GraphicsResources.VertexDesc = GFXDevice->CreateVertexDesc(SpriteVertexAttribs.data(), SpriteVertexAttribs.size(),
				DefaultSpriteResources.DefaultShader.get());
			GraphicsResources.VertexDesc->SetDebugName("[Starshine] SpriteRenderer::SpriteVertexDesc");
		}

		void Internal_CreateIndexBuffer()
		{
			std::unique_ptr<u16[]> indexData = std::make_unique<u16[]>(MaxIndices);

			// NOTE: Vertex order:
			//		 [0] - Top left,  [1] - Bottom right,
			//		 [2] - Top right, [3] - Bottom left
			// 
			//		 Index order:
			//		 [0] - Top left(0),     [1] - Top right(2),
			//		 [2] - Bottom right(1), [3] - Bottom left(3)

			u16 baseVertex = 0;
			for (size_t i = 0; i < MaxIndices; i += 6)
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

			size_t indexBufferSize = MaxIndices * sizeof(u16);
			GraphicsResources.SpriteIndexBuffer = GFXDevice->CreateIndexBuffer(indexBufferSize, IndexFormat::Index16bit, indexData.get(), false);
			GraphicsResources.SpriteIndexBuffer->SetDebugName("[Starshine] SpriteRenderer::SpriteIndexBuffer");
		}
	
		void Internal_CreateBlendStates()
		{
			static constexpr std::array<std::string_view, EnumCount<BlendMode>()> debugBlendStateNames
			{
				"[Starshine] SpriteRenderer::BlendState_Normal",
				"[Starshine] SpriteRenderer::BlendState_Add",
				"[Starshine] SpriteRenderer::BlendState_Multiply",
				"[Starshine] SpriteRenderer::BlendState_Overlay"
			};

			size_t i = 0;
			for (auto& mode : BlendModes)
			{
				mode.StateObject = GFXDevice->CreateBlendState(mode.Desc);
				mode.StateObject->SetDebugName(debugBlendStateNames[i++]);
			}
		}

		void Internal_CreateDefaultSpriteResources()
		{
			DefaultSpriteResources.DefaultShader = Rendering::Utilities::LoadShader("diva/shaders/d3d11/VS_SpriteDefault.cso", "diva/shaders/d3d11/FS_SpriteDefault.cso");
			DefaultSpriteResources.DefaultShader->SetDebugName("[Starshine] SpriteRenderer::DefaultShader");

			static constexpr array<u8, 4> defaultTexData { 0xFF, 0xFF, 0xFF, 0xFF };
			DefaultSpriteResources.DefaultTexture = GFXDevice->CreateTexture(1, 1, TextureFormat::RGBA8, defaultTexData.data());
			DefaultSpriteResources.DefaultTexture->SetDebugName("[Starshine] SpriteRenderer::DefaultTexture");
		}

		void Internal_CreateShaderUniformBuffer()
		{
			GraphicsResources.ShaderUniformBuffer = GFXDevice->CreateUniformBuffer(sizeof(ShaderUniformsBufferData), nullptr, false);
			GraphicsResources.ShaderUniformBuffer->SetDebugName("[Starshine] SpriteRenderer::ShaderUniformBuffer");
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

			if (CurrentList.Texture != listTex)
			{
				if (PushedDrawCommands == 0)
				{
					if (CurrentList.ShapeVertexCount != 0)
					{

					}

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
			ShaderUniforms.TransformMatrix = glm::transpose(glm::orthoRH_ZO(viewportSize.X, viewportSize.Width, viewportSize.Height, viewportSize.Y, 0.0f, 1.0f));

			GraphicsResources.ShaderUniformBuffer->SetData(&ShaderUniforms, 0, sizeof(ShaderUniformsBufferData));
			GFXDevice->SetUniformBuffer(GraphicsResources.ShaderUniformBuffer.get(), ShaderStage::Vertex, 0);

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
					GFXDevice->DrawIndexed(PrimitiveType::Triangles, list->FirstSpriteIndex * 6, 0, list->SpriteCount * 6);
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
			PushedShapeVertices = 0;

			ResetSprite();
			ResetList();
		}

		void PushShape(const SpriteVertex* vertices, size_t vertexCount, PrimitiveType primType, Texture* texture)
		{
			if (PushedDrawCommands + 1 >= MaxLists || PushedShapeVertices + vertexCount >= MaxShapeVertices) { RenderSprites(nullptr); }

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

			CurrentList.ShapeVertexCount += vertexCount;

			size_t capacity = PushedShapeVertices;
			if (capacity < PushedShapeVertices + vertexCount)
			{
				ShapeVertices.reserve(PushedShapeVertices + vertexCount);
			}

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
			const BlendModeState& blendState = BlendModes[static_cast<size_t>(mode)];
			GFXDevice->SetBlendState(blendState.StateObject.get());
		}
	};

	SpriteRenderer::SpriteRenderer() : impl(std::make_unique<Impl>(*this))
	{
	}

	SpriteRenderer::~SpriteRenderer()
	{
		Destroy();
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
