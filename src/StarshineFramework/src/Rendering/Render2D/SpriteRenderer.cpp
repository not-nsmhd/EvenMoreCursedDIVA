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

	namespace
	{
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
		};
	}

	std::vector<std::unique_ptr<BlendState>> BlendStates; // RAII moment

	struct SpriteRenderer::Impl
	{
		Device* GFXDevice{};

		SpriteSheetRenderer SpriteSheetRenderer;
		FontRenderer FontRenderer;
		AnimationSetRenderer AnimationSetRenderer;

		struct ShaderUniformsBufferData
		{
			mat4 TransProjMatrix;
		} ShaderUniforms;

		struct
		{
			std::unique_ptr<Buffer> SpriteVertexBuffer{};
			std::unique_ptr<Buffer> SpriteIndexBuffer{};
			std::unique_ptr<VertexDesc> VertexDesc{};

			std::unique_ptr<Buffer> ShapeVertexBuffer{};

			std::unique_ptr<Buffer> VSUniformBuffer{};
			std::unique_ptr<Buffer> FSUniformBuffer{};
		} GraphicsResources;

		struct
		{
			std::unique_ptr<Shader> DefaultShader{};
			std::unique_ptr<Graphics::Texture> DefaultTexture{};
		} DefaultSpriteResources;

		vector<SpriteState> Sprites;
		vector<SpriteVertex> SpriteVertexData;
		vector<SpriteVertex> ShapeVertexData;

		vector<Command> Commands;

		u32 PushedSprites = 0;
		u32 PushedShapeVertices = 0;

		SpriteState CurrentSprite{};

		Command CurrentCommand{};
		StarshineTex* LastSpriteTexture{};

	public:
		Impl(SpriteRenderer& parent) : SpriteSheetRenderer(parent), FontRenderer(parent), AnimationSetRenderer(parent)
		{
			GFXDevice = Rendering::GetDevice();

			Init_CreateDefaultSpriteResources();
			Init_CreateShaderUniformBuffer();
			Init_CreateBlendStates();

			Init_CreateVertexBuffer();
			Init_CreateIndexBuffer();

			FontRenderer.LoadResources(GFXDevice);

			PushFrameBeginCommands();
		}

		~Impl()
		{
			BlendStates.clear();
		}

		void Init_CreateVertexBuffer()
		{
			BufferCreationData bufferInfo{};
			bufferInfo.Type = BufferType::Vertex;
			bufferInfo.Size = MaxVertices * sizeof(SpriteVertex);
			bufferInfo.Dynamic = true;

			GFXDevice->CreateBuffer(bufferInfo, GraphicsResources.SpriteVertexBuffer);

			bufferInfo.Size = MaxShapeVertices * sizeof(SpriteVertex);
			GFXDevice->CreateBuffer(bufferInfo, GraphicsResources.ShapeVertexBuffer);

			static constexpr array<VertexAttrib, 3> spriteVertexDesc
			{
				VertexAttrib { VertexAttribType::Position, 0, VertexAttribFormat::Float2, sizeof(SpriteVertex), offsetof(SpriteVertex, Position) },
				VertexAttrib { VertexAttribType::TexCoord, 0, VertexAttribFormat::Float2, sizeof(SpriteVertex), offsetof(SpriteVertex, TexCoord) },
				VertexAttrib { VertexAttribType::Color, 0, VertexAttribFormat::UnsignedByte4Norm, sizeof(SpriteVertex), offsetof(SpriteVertex, Color) }
			};

			GFXDevice->CreateVertexDesc(spriteVertexDesc.data(), spriteVertexDesc.size(),
				DefaultSpriteResources.DefaultShader.get(), GraphicsResources.VertexDesc);

#if defined(_DEBUG)
			GraphicsResources.SpriteVertexBuffer->SetDebugName("SpriteRenderer::SpriteVertexBuffer");
			GraphicsResources.ShapeVertexBuffer->SetDebugName("SpriteRenderer::ShapeVertexBuffer");
			GraphicsResources.VertexDesc->SetDebugName("SpriteRenderer::SpriteVertexDesc");
#endif
		}

		void Init_CreateIndexBuffer()
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
	
		void Init_CreateBlendStates()
		{
#if defined (_DEBUG)
			static constexpr std::array<std::string_view, EnumCount<BlendMode>()> debugBlendStateNames
			{
				"SpriteRenderer::BlendState_Normal",
				"SpriteRenderer::BlendState_Add",
				"SpriteRenderer::BlendState_Multiply",

				"SpriteRenderer::BlendState_Disabled"
			};
#endif

			static constexpr std::array<BlendStateDesc, EnumCount<BlendMode>()> descs
			{
				BlendStateDesc
				// Normal
				{ BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add },
				// Add
				{ BlendFactor::SrcAlpha, BlendFactor::One, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add },
				// Multiply
				{ BlendFactor::DestColor, BlendFactor::OneMinusSrcAlpha, BlendFactor::Zero, BlendFactor::One, BlendOperation::Add, BlendOperation::Add },

				// Disabled
				{ BlendFactor::Zero, BlendFactor::Zero, BlendFactor::Zero, BlendFactor::Zero, BlendOperation::Add, BlendOperation::Add },
			};

			BlendStates.resize(descs.size());

			size_t i = 0;
			for (auto& desc : descs)
			{
				GFXDevice->CreateBlendState(desc, BlendStates[i]);
#ifdef _DEBUG
				BlendStates[i]->SetDebugName(debugBlendStateNames[i]);
#endif
				i++;
			}
		}

		void Init_CreateDefaultSpriteResources()
		{
			Rendering::Utilities::LoadShader("shaders/d3d11/VS_SpriteDefault.cso", "shaders/d3d11/FS_SpriteDefault.cso", DefaultSpriteResources.DefaultShader);

			static constexpr u8 defaultTexData[4] { 0xFF, 0xFF, 0xFF, 0xFF };
			DefaultSpriteResources.DefaultTexture = std::make_unique<Graphics::Texture>(vec2{ 1, 1 }, TextureFormat::RGBA8, TextureFlags{}, defaultTexData);
			GFXDevice->UploadTexture(DefaultSpriteResources.DefaultTexture.get());

#ifdef _DEBUG
			DefaultSpriteResources.DefaultTexture->SetName("SpriteRenderer::DefaultTexture");
			DefaultSpriteResources.DefaultShader->SetDebugName("SpriteRenderer::DefaultShader");
#endif
		}

		void Init_CreateShaderUniformBuffer()
		{
			BufferCreationData bufferInfo{};
			bufferInfo.Type = BufferType::Uniform;
			bufferInfo.Size = sizeof(ShaderUniformsBufferData);
			bufferInfo.Dynamic = true;

			GFXDevice->CreateBuffer(bufferInfo, GraphicsResources.VSUniformBuffer);

			bufferInfo.Type = BufferType::Uniform;
			bufferInfo.Size = MaxFSUniformDataSize;
			bufferInfo.Dynamic = true;

			GFXDevice->CreateBuffer(bufferInfo, GraphicsResources.FSUniformBuffer);
#ifdef _DEBUG
			GraphicsResources.VSUniformBuffer->SetDebugName("SpriteRenderer::VSUniformBuffer");
			GraphicsResources.FSUniformBuffer->SetDebugName("SpriteRenderer::FSUniformBuffer");
#endif
		}

		void PushFrameBeginCommands()
		{
			SetShader(nullptr, nullptr, 0);
			SetBlendMode(BlendMode::Normal);
		}

		inline void ResetSprite()
		{
			CurrentSprite = {};

			CurrentSprite.RotationCos = SDL_cosf(0.0f);
			CurrentSprite.RotationSin = SDL_sinf(0.0f);

			CurrentSprite.VertexColors.TopLeft = DefaultColors::White;
			CurrentSprite.VertexColors.TopRight = DefaultColors::White;
			CurrentSprite.VertexColors.BottomLeft = DefaultColors::White;
			CurrentSprite.VertexColors.BottomRight = DefaultColors::White;

			CurrentSprite.SourceRect_TexSpace = { 0.0f, 0.0f, 1.0f, 1.0f };
		}

		inline void PushCurrentCommand()
		{
			if (CurrentCommand.Type == CommandType::DrawSprites || CurrentCommand.Type == CommandType::DrawShape)
			{
				if (FontRenderer.doNotSetShaderOnNextCall)
				{
					FontRenderer.doNotSetShaderOnNextCall = false;
					SetShader(DefaultSpriteResources.DefaultShader.get(), nullptr, 0);
				}

				Commands.push_back(CurrentCommand);
				CurrentCommand = {};
			}
		}

		inline void PushSetTextureCommand(StarshineTex* tex)
		{
			Command& setTexCommand = Commands.emplace_back();
			setTexCommand.Type = CommandType::SetTexture;
			setTexCommand.SetTexture.Texture = tex;
		}

		void PushSprite(StarshineTex* texture)
		{
			if (PushedSprites >= MaxSprites || Commands.size() >= MaxCommands)
				Present();

			Sprites.push_back(CurrentSprite);

			ResetSprite();

			StarshineTex* commandTex = (texture != nullptr) ? texture : DefaultSpriteResources.DefaultTexture.get();

			if (LastSpriteTexture != commandTex || CurrentCommand.Type != CommandType::DrawSprites)
			{
				PushCurrentCommand();
				PushSetTextureCommand(commandTex);

				CurrentCommand.Type = CommandType::DrawSprites;
				CurrentCommand.DrawSprites.FirstSpriteIndex = PushedSprites;
				CurrentCommand.DrawSprites.SpriteCount = 1;

				LastSpriteTexture = commandTex;
			}
			else
				CurrentCommand.DrawSprites.SpriteCount++;

			PushedSprites++;
		}

		void DrawShape(const SpriteVertex* vertices, size_t vertexCount, PrimitiveType primType, Graphics::Texture* texture)
		{
			assert(vertexCount <= MaxShapeVertices);
			if (PushedShapeVertices >= MaxShapeVertices || Commands.size() >= MaxCommands)
				Present();

			for (size_t i = 0; i < vertexCount; i++)
				ShapeVertexData.emplace_back(vertices[i]);

			PushCurrentCommand();
			PushSetTextureCommand((texture != nullptr) ? texture : DefaultSpriteResources.DefaultTexture.get());

			CurrentCommand.Type = CommandType::DrawShape;
			CurrentCommand.DrawShape.FirstVertexIndex = PushedShapeVertices;
			CurrentCommand.DrawShape.VertexCount = vertexCount;
			CurrentCommand.DrawShape.PrimitiveType = primType;
			PushCurrentCommand();

			PushedShapeVertices += vertexCount;
		}

		void SetBlendMode(BlendMode mode)
		{
			PushCurrentCommand();

			Command& setBlendModeCommand = Commands.emplace_back();
			setBlendModeCommand.Type = CommandType::SetBlendMode;
			setBlendModeCommand.SetBlendMode.Mode = mode;
		}

		void SetShader(const Shader* shader, const void* fsData, size_t fsDataSize)
		{
			assert(fsDataSize <= MaxFSUniformDataSize);
				
			PushCurrentCommand();

			Command& command = Commands.emplace_back();
			command.Type = CommandType::SetShader;
			command.SetShader.Shader = (shader == nullptr ? DefaultSpriteResources.DefaultShader.get() : shader);

			if (fsData != nullptr && fsDataSize > 0)
				SDL_memcpy(command.SetShader.FSUniformData, fsData, fsDataSize);
		}

		void SetTransformMatrix(mat4 matrix)
		{
			PushCurrentCommand();

			Command& command = Commands.emplace_back();
			command.Type = CommandType::SetTransformMatrix;
			command.SetTransformMatrix.Matrix = glm::transpose(matrix);
		}

		void GenerateSpriteVertices()
		{
			const size_t spriteVertexCount = static_cast<size_t>(PushedSprites) * 4;
			if (SpriteVertexData.size() < spriteVertexCount)
				SpriteVertexData.resize(spriteVertexCount);

			size_t baseVertex = 0;
			for (auto curSprite = Sprites.cbegin(); curSprite != Sprites.cbegin() + PushedSprites; curSprite++)
			{
				// Top-left
				SpriteVertex* vertex = &SpriteVertexData[baseVertex + 0];
				vec2 basePos = {};

				vertex->Position = MathExtensions::RotateVector(basePos, curSprite->Origin, curSprite->RotationCos, curSprite->RotationSin) + curSprite->Position;
				vertex->TexCoord = { curSprite->SourceRect_TexSpace.X, curSprite->SourceRect_TexSpace.Y };
				vertex->Color = curSprite->VertexColors.TopLeft;

				// Bottom-right
				vertex = &SpriteVertexData[baseVertex + 1];
				basePos = { curSprite->Size.x, curSprite->Size.y };

				vertex->Position = MathExtensions::RotateVector(basePos, curSprite->Origin, curSprite->RotationCos, curSprite->RotationSin) + curSprite->Position;
				vertex->TexCoord = { curSprite->SourceRect_TexSpace.Width, curSprite->SourceRect_TexSpace.Height };
				vertex->Color = curSprite->VertexColors.BottomRight;

				// Top-right
				vertex = &SpriteVertexData[baseVertex + 2];
				basePos = { curSprite->Size.x, 0.0f };

				vertex->Position = MathExtensions::RotateVector(basePos, curSprite->Origin, curSprite->RotationCos, curSprite->RotationSin) + curSprite->Position;
				vertex->TexCoord = { curSprite->SourceRect_TexSpace.Width, curSprite->SourceRect_TexSpace.Y };
				vertex->Color = curSprite->VertexColors.TopRight;

				// Bottom-left
				vertex = &SpriteVertexData[baseVertex + 3];
				basePos = { 0.0f, curSprite->Size.y };

				vertex->Position = MathExtensions::RotateVector(basePos, curSprite->Origin, curSprite->RotationCos, curSprite->RotationSin) + curSprite->Position;
				vertex->TexCoord = { curSprite->SourceRect_TexSpace.X, curSprite->SourceRect_TexSpace.Height };
				vertex->Color = curSprite->VertexColors.BottomLeft;

				baseVertex += 4;
			}

			if (spriteVertexCount > 0)
				GraphicsResources.SpriteVertexBuffer->SetData(SpriteVertexData.data(), 0, spriteVertexCount * sizeof(SpriteVertex));
		}

		inline void Present_Prepare()
		{
			PushCurrentCommand();

			GenerateSpriteVertices();

			if (!ShapeVertexData.empty())
				GraphicsResources.ShapeVertexBuffer->SetData(ShapeVertexData.data(), 0, ShapeVertexData.size() * sizeof(SpriteVertex));
		}

		void Present_End()
		{
			Sprites.clear();
			SpriteVertexData.clear();
			ShapeVertexData.clear();
			Commands.clear();

			PushedSprites = 0;
			PushedShapeVertices = 0;

			ResetSprite();
			LastSpriteTexture = nullptr;

			PushFrameBeginCommands();
		}

		enum class VertexBufferType
		{
			NotSet,
			Sprite,
			Shape
		};

		void Present_ProcessCommands()
		{
			const RectangleF viewportSize = GFXDevice->GetViewportSize();
			const mat4 projMatrix = glm::transpose(glm::ortho(viewportSize.X, viewportSize.Width, viewportSize.Height, viewportSize.Y, 0.0f, 1.0f));
			GraphicsResources.VSUniformBuffer->SetData(glm::value_ptr(projMatrix), 0, sizeof(mat4));

			GFXDevice->SetUniformBuffer(GraphicsResources.VSUniformBuffer.get(), ShaderStage::Vertex, 0);
			GFXDevice->SetUniformBuffer(GraphicsResources.FSUniformBuffer.get(), ShaderStage::Fragment, 0);

			VertexBufferType currentBuffer = VertexBufferType::NotSet;
			for (const auto& command : Commands)
			{
				switch (command.Type)
				{
				case CommandType::DrawSprites:
				{
					if (currentBuffer != VertexBufferType::Sprite)
					{
						GFXDevice->SetVertexBuffer(GraphicsResources.SpriteVertexBuffer.get(), GraphicsResources.VertexDesc.get());
						GFXDevice->SetIndexBuffer(GraphicsResources.SpriteIndexBuffer.get());
						currentBuffer = VertexBufferType::Sprite;
					}

					const u32 baseVertex = command.DrawSprites.FirstSpriteIndex * 4;
					const u32 indexCount = command.DrawSprites.SpriteCount * 6;
					GFXDevice->DrawIndexed(PrimitiveType::Triangles, 0, baseVertex, indexCount);
					break;
				}
				case CommandType::DrawShape:
				{
					if (currentBuffer != VertexBufferType::Shape)
					{
						GFXDevice->SetVertexBuffer(GraphicsResources.ShapeVertexBuffer.get(), GraphicsResources.VertexDesc.get());
						GFXDevice->SetIndexBuffer(nullptr);
						currentBuffer = VertexBufferType::Shape;
					}

					GFXDevice->DrawArrays(command.DrawShape.PrimitiveType, command.DrawShape.FirstVertexIndex, command.DrawShape.VertexCount);
					break;
				}
				case CommandType::SetTexture:
				{
					GFXDevice->SetTexture(command.SetTexture.Texture, 0);
					break;
				}
				case CommandType::SetTransformMatrix:
				{
					const mat4 transProjMatrix = command.SetTransformMatrix.Matrix * projMatrix;
					GraphicsResources.VSUniformBuffer->SetData(glm::value_ptr(transProjMatrix), 0, sizeof(mat4));
					break;
				}
				case CommandType::SetShader:
				{
					GraphicsResources.FSUniformBuffer->SetData(command.SetShader.FSUniformData, 0, MaxFSUniformDataSize);
					GFXDevice->SetShader(command.SetShader.Shader);
					break;
				}
				case CommandType::SetBlendMode:
				{
					if (command.SetBlendMode.Mode == BlendMode::Disabled)
						GFXDevice->SetBlendState(nullptr);
					else
					{
						const size_t index = static_cast<size_t>(command.SetBlendMode.Mode);
						GFXDevice->SetBlendState(BlendStates[index].get());
					}
					break;
				}
				}
			}

			GFXDevice->SetUniformBuffer(nullptr, ShaderStage::Vertex, 0);
			GFXDevice->SetUniformBuffer(nullptr, ShaderStage::Fragment, 0);
		}

		void Present()
		{
			if (PushedSprites == 0 && PushedShapeVertices == 0)
				return;

			Present_Prepare();
			Present_ProcessCommands();
			Present_End();
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

	void SpriteRenderer::SetSpriteSource(RectangleF texSpaceSource)
	{
		impl->CurrentSprite.SourceRect_TexSpace = texSpaceSource;
	}

	void SpriteRenderer::SetSpriteSource(const StarshineTex* texture, RectangleF absSource)
	{
		const ivec2 texSize = texture->GetSize();

		f32 w = (texSize.x > 0) ? static_cast<f32>(texSize.x) : 1.0f;
		f32 h = (texSize.y > 0) ? static_cast<f32>(texSize.y) : 1.0f;

		impl->CurrentSprite.SourceRect_TexSpace.X = absSource.X / w;
		impl->CurrentSprite.SourceRect_TexSpace.Width = (absSource.X + absSource.Width) / w;
		impl->CurrentSprite.SourceRect_TexSpace.Y = absSource.Y / h;
		impl->CurrentSprite.SourceRect_TexSpace.Height = (absSource.Y + absSource.Height) / h;
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

	void SpriteRenderer::SetShader(const Shader* shader, const void* fsData, size_t fsDataSize)
	{
		impl->SetShader(shader, fsData, fsDataSize);
	}

	void SpriteRenderer::SetTransformMatrix(mat4 matrix)
	{
		impl->SetTransformMatrix(matrix);
	}

	void SpriteRenderer::PushSprite(StarshineTex* texture)
	{
		impl->PushSprite(texture);
	}

	void SpriteRenderer::DrawShape(const SpriteVertex* vertices, size_t vertexCount, PrimitiveType primType, StarshineTex* texture)
	{
		impl->DrawShape(vertices, vertexCount, primType, texture);
	}

	void SpriteRenderer::Present()
	{
		impl->Present();
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

	size_t SpriteRenderer::GetCommands(Command** commands)
	{
#ifdef _DEBUG
		if (commands != nullptr)
			*commands = impl->Commands.data();
		return impl->Commands.size();
#endif // _DEBUG
		return 0;
	}
};
