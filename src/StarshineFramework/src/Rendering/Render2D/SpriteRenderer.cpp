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
	using namespace Graphics;
	using StarshineTex = Graphics::Texture;

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
		StarshineTex* Texture = nullptr;
	};

	constexpr array<VertexAttrib, 3> SpriteVertexAttribs
	{
		VertexAttrib { VertexAttribType::Position, 0, VertexAttribFormat::Float2, sizeof(SpriteVertex), offsetof(SpriteVertex, Position) },
		VertexAttrib { VertexAttribType::TexCoord, 0, VertexAttribFormat::Float2, sizeof(SpriteVertex), offsetof(SpriteVertex, TexCoord) },
		VertexAttrib { VertexAttribType::Color, 0, VertexAttribFormat::UnsignedByte4Norm, sizeof(SpriteVertex), offsetof(SpriteVertex, Color) }
	};

	constexpr std::array<BlendStateDesc, EnumCount<BlendMode>()> BlendModeDescs
	{
		BlendStateDesc
		{ BlendFactor::Zero, BlendFactor::Zero, BlendFactor::Zero, BlendFactor::Zero, BlendOperation::Add, BlendOperation::Add },
		{ BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add },
		{ BlendFactor::SrcAlpha, BlendFactor::One, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add },
		{ BlendFactor::DestColor, BlendFactor::Zero, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add }
	};

	std::vector<std::unique_ptr<BlendState>> BlendStates; // RAII moment

	struct SpriteRenderer::Impl
	{
		Device* GFXDevice{};

		SpriteSheetRenderer SpriteSheetRenderer;
		FontRenderer FontRenderer;
		AnimationSetRenderer AnimationSetRenderer;

		struct ShaderUniformsBufferData
		{
			mat4 TransformMatrix;
		} ShaderUniforms;

		vec2 BasePosition{};
		vec2 BaseScale{ 1.0f, 1.0f };

		struct
		{
			std::unique_ptr<Buffer> SpriteVertexBuffer{};
			std::unique_ptr<Buffer> SpriteIndexBuffer{};
			std::unique_ptr<VertexDesc> VertexDesc{};

			std::unique_ptr<Buffer> ShapeVertexBuffer{};

			std::unique_ptr<Buffer> ShaderUniformBuffer{};
		} GraphicsResources;

		struct
		{
			std::unique_ptr<Shader> DefaultShader{};
			std::unique_ptr<Graphics::Texture> DefaultTexture{};
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
		Impl(SpriteRenderer& parent) : SpriteSheetRenderer(parent), FontRenderer(parent), AnimationSetRenderer(parent)
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
			BlendStates.clear();
		}

		void Internal_CreateVertexBuffer()
		{
			BufferCreationData bufferInfo{};
			bufferInfo.Type = BufferType::Vertex;
			bufferInfo.Size = MaxVertices * sizeof(SpriteVertex);
			bufferInfo.Dynamic = true;

			GFXDevice->CreateBuffer(bufferInfo, GraphicsResources.SpriteVertexBuffer);

			bufferInfo.Size = MaxShapeVertices * sizeof(SpriteVertex);
			GFXDevice->CreateBuffer(bufferInfo, GraphicsResources.ShapeVertexBuffer);

			GFXDevice->CreateVertexDesc(SpriteVertexAttribs.data(), SpriteVertexAttribs.size(), 
				DefaultSpriteResources.DefaultShader.get(), GraphicsResources.VertexDesc);

#if defined(_DEBUG)
			GraphicsResources.SpriteVertexBuffer->SetDebugName("SpriteRenderer::SpriteVertexBuffer");
			GraphicsResources.ShapeVertexBuffer->SetDebugName("SpriteRenderer::ShapeVertexBuffer");
			GraphicsResources.VertexDesc->SetDebugName("SpriteRenderer::SpriteVertexDesc");
#endif
		}

		void Internal_CreateIndexBuffer()
		{
			std::unique_ptr<u16[]> indexData = std::make_unique<u16[]>(MaxIndices);

			// NOTE: Vertex order:
			//		 [0] - Top left,  [1] - Bottom right,
			//		 [2] - Top right, [3] - Bottom left
			// 
			//		 Index order:
			//		 [0] - Top left(0),     [1] - Top right(2),     [2] - Bottom right(1)   (First triangle)
			//		 [3] - Bottom right(1), [4] - Bottom left(3)    [5] - Top left(0)       (Second triangle)

			u16 baseVertex = 0;
			for (size_t i = 0; i < MaxIndices; i += 6)
			{
				// NOTE: Indices are always arranged in clockwise order regardless of backend
				// (OpenGL is switchted to clockwise order on initialization, D3D uses clockwise order by default)
				indexData[i + 0] = baseVertex + 0;
				indexData[i + 1] = baseVertex + 2;
				indexData[i + 2] = baseVertex + 1;
				indexData[i + 3] = baseVertex + 1;
				indexData[i + 4] = baseVertex + 3;
				indexData[i + 5] = baseVertex + 0;

				baseVertex += 4;
			}

			BufferCreationData bufferInfo{};
			bufferInfo.Type = BufferType::Index;
			bufferInfo.IndexFormat = IndexFormat::Index16bit;
			bufferInfo.Size = MaxIndices * sizeof(u16);
			bufferInfo.InitialData = indexData.get();

			GFXDevice->CreateBuffer(bufferInfo, GraphicsResources.SpriteIndexBuffer);
			
#if defined (_DEBUG)
			GraphicsResources.SpriteIndexBuffer->SetDebugName("SpriteRenderer::SpriteIndexBuffer");
#endif
		}
	
		void Internal_CreateBlendStates()
		{
#if defined (_DEBUG)
			static constexpr std::array<std::string_view, EnumCount<BlendMode>()> debugBlendStateNames
			{
				"SpriteRenderer::BlendState_Disabled",
				"SpriteRenderer::BlendState_Normal",
				"SpriteRenderer::BlendState_Add",
				"SpriteRenderer::BlendState_Multiply"
			};
#endif

			BlendStates.resize(BlendModeDescs.size());

			size_t i = 0;
			for (auto& desc : BlendModeDescs)
			{
				GFXDevice->CreateBlendState(desc, BlendStates[i]);
#ifdef _DEBUG
				BlendStates[i]->SetDebugName(debugBlendStateNames[i]);
#endif
				i++;
			}
		}

		void Internal_CreateDefaultSpriteResources()
		{
			Rendering::Utilities::LoadShader("diva/shaders/d3d11/VS_SpriteDefault.cso", "diva/shaders/d3d11/FS_SpriteDefault.cso", DefaultSpriteResources.DefaultShader);

			static constexpr u8 defaultTexData[4] { 0xFF, 0xFF, 0xFF, 0xFF };
			DefaultSpriteResources.DefaultTexture = std::make_unique<Graphics::Texture>(vec2{ 1, 1 }, TextureFormat::RGBA8, TextureFlags{}, defaultTexData);
			GFXDevice->UploadTexture(DefaultSpriteResources.DefaultTexture.get());

#ifdef _DEBUG
			DefaultSpriteResources.DefaultTexture->SetName("SpriteRenderer::DefaultTexture");
			DefaultSpriteResources.DefaultShader->SetDebugName("SpriteRenderer::DefaultShader");
#endif
		}

		void Internal_CreateShaderUniformBuffer()
		{
			BufferCreationData bufferInfo{};
			bufferInfo.Type = BufferType::Uniform;
			bufferInfo.Size = sizeof(ShaderUniformsBufferData);
			bufferInfo.Dynamic = true;

			GFXDevice->CreateBuffer(bufferInfo, GraphicsResources.ShaderUniformBuffer);
#ifdef _DEBUG
			GraphicsResources.ShaderUniformBuffer->SetDebugName("SpriteRenderer::ShaderUniformBuffer");
#endif
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

		void PushSprite(StarshineTex* texture)
		{
			if (PushedSprites >= MaxSprites)
			{
				RenderSprites(nullptr, true);
			}

			PushedSprites++;
			Sprites.push_back(CurrentSprite);

			ResetSprite();

			StarshineTex* listTex = (texture != nullptr) ? texture : DefaultSpriteResources.DefaultTexture.get();

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

		void RenderSprites(Shader* shader, bool doNotResetBaseValues)
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
			viewportSize.X += BasePosition.x;
			viewportSize.Y += BasePosition.y;
			viewportSize.Width /= BaseScale.x;
			viewportSize.Height /= BaseScale.y;
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

			if (!doNotResetBaseValues)
			{
				BasePosition = {};
				BaseScale = vec2(1.0f);
			}
		}

		void PushShape(const SpriteVertex* vertices, size_t vertexCount, PrimitiveType primType, StarshineTex* texture)
		{
			if (PushedDrawCommands + 1 >= MaxLists || PushedShapeVertices + vertexCount >= MaxShapeVertices)
				RenderSprites(nullptr, true);

			StarshineTex* listTex = (texture != nullptr) ? texture : DefaultSpriteResources.DefaultTexture.get();

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
			if (mode == BlendMode::Disabled)
			{
				GFXDevice->SetBlendState(nullptr);
			}
			else
			{
				GFXDevice->SetBlendState(BlendStates[static_cast<size_t>(mode)].get());
			}
		}
	};

	SpriteRenderer::SpriteRenderer() : impl(std::make_unique<Impl>(*this))
	{
	}

	SpriteRenderer::~SpriteRenderer()
	{
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

	void SpriteRenderer::SetSpriteSource(const StarshineTex* texture, const RectangleF& absSource)
	{
		const ivec2 texSize = texture->GetSize();

		f32 w = (texSize.x > 0) ? static_cast<f32>(texSize.x) : 1.0f;
		f32 h = (texSize.y > 0) ? static_cast<f32>(texSize.y) : 1.0f;

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

	void SpriteRenderer::PushSprite(StarshineTex* texture)
	{
		impl->PushSprite(texture);
	}

	void SpriteRenderer::SetBasePositionAndScale(const vec2& pos, const vec2& scale)
	{
		impl->BasePosition = pos;
		impl->BaseScale = scale;
	}

	void SpriteRenderer::GetBasePositionAndScale(vec2& pos, vec2& scale)
	{
		pos = impl->BasePosition;
		scale = impl->BaseScale;
	}

	void SpriteRenderer::RenderSprites(Shader* shader)
	{
		impl->RenderSprites(shader, false);
	}

	void SpriteRenderer::PushShape(const SpriteVertex* vertices, size_t vertexCount, PrimitiveType primType, StarshineTex* texture)
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

	AnimationSetRenderer& SpriteRenderer::AnimationSet()
	{
		return impl->AnimationSetRenderer;
	}
};
