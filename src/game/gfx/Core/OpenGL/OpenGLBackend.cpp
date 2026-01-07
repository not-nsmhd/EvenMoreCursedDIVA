#include <glad/glad.h>
#include "OpenGLBackend.h"
#include <array>
#include <vector>
#include "io/Xml.h"
#include "io/File.h"
#include "common/math_ext.h"
#include "util/logging.h"

namespace Starshine::GFX::Core::OpenGL
{
	using namespace Logging;
	using namespace Common;
	using std::array;
	using std::vector;
	using std::string_view;

	constexpr const char* LogName = "Starshine::GFX::Core::OpenGL";

	namespace ConversionTables
	{
		constexpr array<GLenum, EnumCount<PrimitiveType>()> GLPrimitiveTypes =
		{
			GL_POINTS,
			GL_LINES,
			GL_LINE_STRIP,
			GL_TRIANGLES,
			GL_TRIANGLE_STRIP
		};

		constexpr array<GLenum, EnumCount<IndexFormat>()> GLIndexFormats =
		{
			GL_UNSIGNED_SHORT,
			GL_UNSIGNED_INT
		};

		constexpr array<GLenum, EnumCount<ShaderType>()> GLShaderTypes =
		{
			GL_VERTEX_PROGRAM_ARB,
			GL_FRAGMENT_PROGRAM_ARB
		};

		constexpr array<GLenum, EnumCount<TextureFormat>()> GLTextureDataFormats =
		{
			GL_RGBA,
			2,
			GL_RED
		};

		constexpr array<GLenum, EnumCount<TextureFormat>()> GLTextureDisplayFormats =
		{
			GL_RGBA,
			GL_LUMINANCE_ALPHA,
			GL_RED
		};

		constexpr array<GLenum, EnumCount<BlendFactor>()> GLBlendFactors =
		{
			GL_ZERO,
			GL_ONE,
			GL_SRC_COLOR,
			GL_ONE_MINUS_SRC_COLOR,
			GL_DST_COLOR,
			GL_ONE_MINUS_DST_COLOR,
			GL_SRC_ALPHA,
			GL_ONE_MINUS_SRC_ALPHA,
			GL_DST_ALPHA,
			GL_ONE_MINUS_DST_ALPHA
		};

		constexpr array<GLenum, EnumCount<BlendOperation>()> GLBlendOperations =
		{
			GL_FUNC_ADD,
			GL_FUNC_SUBTRACT,
			GL_FUNC_REVERSE_SUBTRACT,
			GL_MIN,
			GL_MAX
		};
	}

	struct ResourceContext
	{
		GLuint BaseResourceHandle = 0;
		size_t Size = 0;
	};

	struct VertexBuffer_OpenGL : public VertexBuffer
	{
	public:
		VertexBuffer_OpenGL(ResourceHandle handle, size_t size, bool dynamic)
			: VertexBuffer(handle), Size(size), Dynamic(dynamic) {}

		OpenGLBackend* Backend = nullptr;

		size_t Size = 0;
		bool Dynamic = false;

		void SetData(void* source, size_t offset, size_t size);
	};

	struct IndexBuffer_OpenGL : public IndexBuffer
	{
	public:
		IndexBuffer_OpenGL(ResourceHandle handle, size_t size, IndexFormat format, bool dynamic)
			: IndexBuffer(handle), Size(size), Format(format), Dynamic(dynamic) {}

		OpenGLBackend* Backend = nullptr;

		size_t Size = 0;
		bool Dynamic = false;
		IndexFormat Format = {};

		void SetData(void* source, size_t offset, size_t size);
	};

	struct Shader_OpenGL : public Shader
	{
	public:
		Shader_OpenGL(ResourceHandle handle) : Shader(handle) {}

		OpenGLBackend* Backend = nullptr;

		ResourceHandle VertexHandle = InvalidResourceHandle;
		ResourceHandle FragmentHandle = InvalidResourceHandle;

		std::vector<ShaderVariable> Variables;
		bool UpdateVariables = false;

		void AddVariable(ShaderVariable& variable);

		ShaderVariableIndex GetVariableIndex(std::string_view name);
		void SetVariableValue(ShaderVariableIndex varIndex, void* value);
	};

	struct VertexDesc_OpenGL : public VertexDesc
	{
	public:
		VertexDesc_OpenGL(ResourceHandle handle, const VertexAttrib* attribs, size_t attribCount)
			: VertexDesc(handle), GLAttribs(attribCount) {}

		std::vector<VertexAttrib_OpenGL> GLAttribs;
	};

	struct Texture_OpenGL : public Texture
	{
	public:
		Texture_OpenGL(ResourceHandle handle, u32 width, u32 height, TextureFormat format, bool clamp, bool nearestFilter)
			: Texture(handle),
			Width(width), Height(height), Format(format), Clamp(clamp), NearestFilter(nearestFilter) {}

		OpenGLBackend* Backend = nullptr;

		u32 Width = 0;
		u32 Height = 0;
		TextureFormat Format{};

		bool NearestFilter = false;
		bool Clamp = false;

		u32 GetWidth() const;
		u32 GetHeight() const;

		void SetData(u32 x, u32 y, u32 width, u32 height, const void* data);
	};

	struct OpenGLBackend::Impl
	{
		SDL_Window* GameWindow = nullptr;
		SDL_GLContext GLContext{};

		vector<ResourceContext> ResourceContexts;

		bool VertexBufferSet = false;
		bool VertexDescSet = false;

		bool IndexBufferSet = false;
		GLenum CurrentIndexFormat = 0;

		bool ShaderSet = false;

		bool Initialize(SDL_Window* gameWindow)
		{
			u32 windowFlags = SDL_GetWindowFlags(gameWindow);
			if ((windowFlags & SDL_WINDOW_OPENGL) != SDL_WINDOW_OPENGL)
			{
				LogError(LogName, "Game window is not an OpenGL window (SDL_WINDOW_OPENGL was not specified in SDL_CreateWindow)");
				return false;
			}

			GameWindow = gameWindow;
			if ((GLContext = SDL_GL_CreateContext(GameWindow)) == NULL)
			{
				char message[512] = {};
				SDL_snprintf(message, 511, "Failed to create an OpenGL context.\nError: %s", SDL_GetError());

				LogError(LogName, message);
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (GFX)", message, GameWindow);

				return false;
			}

			if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
			{
				LogError(LogName, "(GLAD) Failed to load OpenGL functions");
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (GLAD)", "Failed to load OpenGL functions", GameWindow);
				SDL_GL_DeleteContext(GLContext);
				return false;
			}

			//SDL_GL_SetSwapInterval(1);

			LogInfo(LogName, "OpenGL Version: %s", glGetString(GL_VERSION));
			LogInfo(LogName, "OpenGL Renderer: %s", glGetString(GL_RENDERER));

			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			glEnable(GL_VERTEX_PROGRAM_ARB);
			glEnable(GL_FRAGMENT_PROGRAM_ARB);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			return true;
		}

		void Destroy()
		{
			SDL_GL_DeleteContext(GLContext);
		}

		void SwapBuffers()
		{
			SDL_GL_SwapWindow(GameWindow);
		}

		void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
		{
			if (VertexBufferSet && VertexDescSet && ShaderSet)
			{
				GLenum primType = ConversionTables::GLPrimitiveTypes[static_cast<size_t>(type)];
				glDrawArrays(primType, firstVertex, vertexCount);
			}
		}

		void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 indexCount)
		{
			if (VertexBufferSet && IndexBufferSet && VertexDescSet && ShaderSet)
			{
				GLenum primType = ConversionTables::GLPrimitiveTypes[static_cast<size_t>(type)];
				size_t firstIndexPointer = static_cast<size_t>(firstIndex) * ((CurrentIndexFormat == GL_UNSIGNED_SHORT) ? 2 : 4);

				glDrawElements(primType, indexCount, CurrentIndexFormat, (const void*)firstIndexPointer);
			}
		}

		void SetVertexBuffer(const VertexBuffer_OpenGL* buffer)
		{
			if (buffer == nullptr)
			{
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
				VertexBufferSet = false;
			}
			else
			{
				GLuint glBufferHandle = ResourceContexts[static_cast<size_t>(buffer->Handle)].BaseResourceHandle;
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, glBufferHandle);
				VertexBufferSet = true;
			}
		}

		void SetIndexBuffer(const IndexBuffer_OpenGL* buffer)
		{
			if (buffer == nullptr)
			{
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
				IndexBufferSet = false;
			}
			else
			{
				GLuint glBufferHandle = ResourceContexts[static_cast<size_t>(buffer->Handle)].BaseResourceHandle;
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, glBufferHandle);
				CurrentIndexFormat = ConversionTables::GLIndexFormats[static_cast<size_t>(buffer->Format)];
				IndexBufferSet = true;
			}
		}

		void SetVertexDesc(const VertexDesc_OpenGL* desc)
		{
			if (desc != nullptr)
			{
				for (auto& attrib : desc->GLAttribs)
				{
					switch (attrib.Type)
					{
					case VertexAttribType::Position:
						glVertexPointer(attrib.Components, attrib.Format, attrib.VertexSize, (const void*)attrib.Offset);
						break;
					case VertexAttribType::Color:
						glColorPointer(attrib.Components, attrib.Format, attrib.VertexSize, (const void*)attrib.Offset);
						break;
					case VertexAttribType::TexCoord:
						glTexCoordPointer(attrib.Components, attrib.Format, attrib.VertexSize, (const void*)attrib.Offset);
						break;
					}
				}

				VertexDescSet = true;
			}
		}

		inline void SetShaderVariableValue(ShaderType shaderType, u32 index, float x, float y, float z, float w)
		{
			GLenum glShaderType = ConversionTables::GLShaderTypes[static_cast<size_t>(shaderType)];
			glProgramLocalParameter4fARB(glShaderType, index, x, y, z, w);
		}

		inline void SetShaderVariableValuePtr(ShaderType shaderType, u32 index, const float* value)
		{
			GLenum glShaderType = ConversionTables::GLShaderTypes[static_cast<size_t>(shaderType)];
			glProgramLocalParameter4fvARB(glShaderType, index, value);
		}

		void SetShader(Shader_OpenGL* shader)
		{
			if (shader == nullptr)
			{
				glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
				ShaderSet = false;
			}
			else
			{
				GLuint vertHandle = ResourceContexts[static_cast<size_t>(shader->VertexHandle)].BaseResourceHandle;
				GLuint fragHandle = ResourceContexts[static_cast<size_t>(shader->FragmentHandle)].BaseResourceHandle;

				glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertHandle);
				glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragHandle);

				if (shader->UpdateVariables)
				{
					for (size_t i = 0; i < shader->Variables.size(); i++)
					{
						const ShaderVariable& variable = shader->Variables.at(i);

						if (variable.Value == nullptr)
						{
							continue;
						}

						switch (variable.Type)
						{
						case ShaderVariableType::Float:
						{
							float* value = reinterpret_cast<float*>(variable.Value);

							SetShaderVariableValue(variable.LocationShader, variable.LocationIndex, *value, 0.0f, 0.0f, 0.0f);
							break;
						}
						case ShaderVariableType::Vector2:
						{
							vec2* value = reinterpret_cast<vec2*>(variable.Value);

							SetShaderVariableValue(variable.LocationShader, variable.LocationIndex, value->x, value->y, 0.0f, 0.0f);
							break;
						}
						case ShaderVariableType::Vector3:
						{
							vec3* value = reinterpret_cast<vec3*>(variable.Value);

							SetShaderVariableValue(variable.LocationShader, variable.LocationIndex, value->x, value->y, value->z, 0.0f);
							break;
						}
						case ShaderVariableType::Vector4:
						{
							vec4* value = reinterpret_cast<vec4*>(variable.Value);

							SetShaderVariableValue(variable.LocationShader, variable.LocationIndex, value->x, value->y, value->z, value->w);
							break;
						}
						case ShaderVariableType::Matrix4:
						{
							// HACK: this is dumb
							mat4* originalMatrix = reinterpret_cast<mat4*>(variable.Value);
							mat4 transposedMatrix = glm::transpose(*originalMatrix);

							SetShaderVariableValuePtr(variable.LocationShader, variable.LocationIndex, reinterpret_cast<const float*>(&transposedMatrix[0]));
							SetShaderVariableValuePtr(variable.LocationShader, variable.LocationIndex + 1, reinterpret_cast<const float*>(&transposedMatrix[1]));
							SetShaderVariableValuePtr(variable.LocationShader, variable.LocationIndex + 2, reinterpret_cast<const float*>(&transposedMatrix[2]));
							SetShaderVariableValuePtr(variable.LocationShader, variable.LocationIndex + 3, reinterpret_cast<const float*>(&transposedMatrix[3]));
							break;
						}
						}
					}

					shader->UpdateVariables = false;
				}

				ShaderSet = true;
			}
		}

		void SetTexture(const Texture_OpenGL* texture, u32 slot)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			if (texture == nullptr)
			{
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else
			{
				GLuint glTexture = ResourceContexts[static_cast<size_t>(texture->Handle)].BaseResourceHandle;
				glBindTexture(GL_TEXTURE_2D, glTexture);
			}
		}

		ResourceHandle FindFreeResourceContext()
		{
			for (size_t i = 0; i < ResourceContexts.size(); i++)
			{
				ResourceContext& ctx = ResourceContexts[i];

				if (ctx.BaseResourceHandle == 0 && ctx.Size == 0)
				{
					return i;
				}
			}

			ResourceContexts.emplace_back();
			return static_cast<ResourceHandle>(ResourceContexts.size() - 1);
		}
	};

	OpenGLBackend::OpenGLBackend() : impl(new OpenGLBackend::Impl())
	{
	}

	OpenGLBackend::~OpenGLBackend()
	{
	}

	bool OpenGLBackend::Initialize(SDL_Window* gameWindow)
	{
		return impl->Initialize(gameWindow);
	}

	void OpenGLBackend::Destroy()
	{
		impl->Destroy();
		delete impl;
	}

	RendererBackendType OpenGLBackend::GetType() const
	{
		return RendererBackendType::OpenGL;
	}

	ResourceContext* OpenGLBackend::GetResourceContext(ResourceHandle handle)
	{
		if (handle != InvalidResourceHandle)
		{
			return &impl->ResourceContexts.at(static_cast<size_t>(handle));
		}
		return nullptr;
	}

	Common::RectangleF OpenGLBackend::GetViewportSize() const
	{
		RectangleF viewport = {};
		glGetFloatv(GL_VIEWPORT, &viewport.X);
		return viewport;
	}

	void OpenGLBackend::Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil)
	{
		GLenum clearFlags = 0;

		if ((flags & ClearFlags::ClearFlags_Color) != 0)
		{
			glClearColor(
				static_cast<float>(color.R) / 255.0f,
				static_cast<float>(color.G) / 255.0f,
				static_cast<float>(color.B) / 255.0f,
				static_cast<float>(color.A) / 255.0f);

			clearFlags |= GL_COLOR_BUFFER_BIT;
		}

		if ((flags & ClearFlags::ClearFlags_Depth) != 0)
		{
			glClearDepth(static_cast<double>(depth));
			clearFlags |= GL_DEPTH_BUFFER_BIT;
		}

		if ((flags & ClearFlags::ClearFlags_Stencil) != 0)
		{
			glClearStencil(stencil);
			clearFlags |= GL_STENCIL_BUFFER_BIT;
		}

		glClear(clearFlags);
	}

	void OpenGLBackend::SwapBuffers()
	{
		impl->SwapBuffers();
	}

	void OpenGLBackend::SetBlendState(bool enable, BlendFactor srcColor, BlendFactor destColor, BlendFactor srcAlpha, BlendFactor destAlpha)
	{
		GLboolean blendEnabled = glIsEnabled(GL_BLEND);
		if (enable)
		{
			if (!blendEnabled)
			{
				glEnable(GL_BLEND);
			}
		}
		else
		{
			if (blendEnabled)
			{
				glDisable(GL_BLEND);
			}
		}

		GLenum glSrcColor = ConversionTables::GLBlendFactors[static_cast<size_t>(srcColor)];
		GLenum glDstColor = ConversionTables::GLBlendFactors[static_cast<size_t>(destColor)];
		GLenum glSrcAlpha = ConversionTables::GLBlendFactors[static_cast<size_t>(srcAlpha)];
		GLenum glDstAlpha = ConversionTables::GLBlendFactors[static_cast<size_t>(destAlpha)];

		glBlendFuncSeparate(glSrcColor, glDstColor, glSrcAlpha, glDstAlpha);
	}

	void OpenGLBackend::SetBlendOperation(BlendOperation op)
	{
		GLenum glOperation = ConversionTables::GLBlendOperations[static_cast<size_t>(op)];
		glBlendEquation(glOperation);
	}

	void OpenGLBackend::DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
	{
		impl->DrawArrays(type, firstVertex, vertexCount);
	}

	void OpenGLBackend::DrawIndexed(PrimitiveType type, u32 firstIndex, u32 indexCount)
	{
		impl->DrawIndexed(type, firstIndex, indexCount);
	}

	VertexBuffer* OpenGLBackend::CreateVertexBuffer(size_t size, void* initialData, bool dynamic)
	{
		if (!dynamic && initialData == nullptr || size == 0)
		{
			return nullptr;
		}

		GLsizeiptr bufferLength = static_cast<GLsizeiptr>(size);
		GLenum bufferUsage = (dynamic) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB;

		GLuint baseBuffer = 0;
		glGenBuffersARB(1, &baseBuffer);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, baseBuffer);

		if (initialData != nullptr)
		{
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, bufferLength, initialData, bufferUsage);
		}
		else
		{
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, bufferLength, NULL, bufferUsage);
		}

		ResourceHandle handle = impl->FindFreeResourceContext();
		ResourceContext* ctx = &impl->ResourceContexts.at(static_cast<size_t>(handle));

		ctx->BaseResourceHandle = baseBuffer;
		ctx->Size = size;

		VertexBuffer_OpenGL* buffer = new VertexBuffer_OpenGL(handle, size, dynamic);
		buffer->Backend = this;

		LogInfo(LogName, "Created a new vertex buffer (handle %d)", handle);
		return buffer;
	}

	// TODO: Make this less copy-pasty
	IndexBuffer* OpenGLBackend::CreateIndexBuffer(size_t size, IndexFormat format, void* initialData, bool dynamic)
	{
		if (!dynamic && initialData == nullptr || size == 0)
		{
			return nullptr;
		}

		GLsizeiptr bufferLength = static_cast<GLsizeiptr>(size);
		GLenum bufferUsage = (dynamic) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB;

		GLuint baseBuffer = 0;
		glGenBuffersARB(1, &baseBuffer);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, baseBuffer);

		if (initialData != nullptr)
		{
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferLength, initialData, bufferUsage);
		}
		else
		{
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferLength, NULL, bufferUsage);
		}

		ResourceHandle handle = impl->FindFreeResourceContext();
		ResourceContext* ctx = &impl->ResourceContexts.at(static_cast<size_t>(handle));

		ctx->BaseResourceHandle = baseBuffer;
		ctx->Size = size;

		IndexBuffer_OpenGL* buffer = new IndexBuffer_OpenGL(handle, size, format, dynamic);
		buffer->Backend = this;

		LogInfo(LogName, "Created a new index buffer (handle %d)", handle);
		return buffer;
	}

	VertexDesc* OpenGLBackend::CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount)
	{
		if (attribs == nullptr || attribCount == 0)
		{
			return nullptr;
		}

		VertexDesc_OpenGL* desc = new VertexDesc_OpenGL(InvalidResourceHandle, attribs, attribCount);

		for (size_t i = 0; i < attribCount; i++)
		{
			const VertexAttrib& gfxAttrib = attribs[i];
			VertexAttrib_OpenGL& glAttrib = desc->GLAttribs[i];

			glAttrib.Type = gfxAttrib.Type;
			glAttrib.Index = gfxAttrib.Index;
			glAttrib.Format = ConversionTables::GLVertexAttribFormat[static_cast<size_t>(gfxAttrib.Format)];
			glAttrib.Components = gfxAttrib.Components;
			glAttrib.VertexSize = gfxAttrib.VertexSize;
			glAttrib.Offset = gfxAttrib.Offset;
		}

		LogInfo(LogName, "Created a new vertex description");
		return desc;
	}

	Shader* OpenGLBackend::LoadShader(const u8* vsData, size_t vsSize, const u8* fsData, size_t fsSize)
	{
		if (vsData == nullptr || vsSize == 0 || fsData == nullptr || fsSize == 0)
		{
			return nullptr;
		}

		GLuint vsHandle = 0;
		GLuint fsHandle = 0;

		u8* vsSource = new u8[vsSize];
		SDL_memcpy(vsSource, vsData, vsSize);

		u8* fsSource = new u8[fsSize];
		SDL_memcpy(fsSource, fsData, fsSize);

		glGenProgramsARB(1, &vsHandle);
		glGenProgramsARB(1, &fsHandle);

		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vsHandle);
		glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, vsSize, vsSource);

		GLint errorPos = -1;
		const GLubyte* errorString = nullptr;

		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (errorPos != -1)
		{
			errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LogError(LogName, "Failed to assemble vertex program. Error (pos: %d): %s", errorPos, errorString);
		}

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fsHandle);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, fsSize, fsSource);

		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (errorPos != -1)
		{
			errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LogError(LogName, "Failed to assemble fragment program. Error (pos: %d): %s", errorPos, errorString);
		}

		ResourceHandle vertHandle = impl->FindFreeResourceContext();
		ResourceContext* vertCtx = &impl->ResourceContexts.at(static_cast<size_t>(vertHandle));

		vertCtx->BaseResourceHandle = vsHandle;
		vertCtx->Size = vsSize;

		ResourceHandle fragHandle = impl->FindFreeResourceContext();
		ResourceContext* fragCtx = &impl->ResourceContexts.at(static_cast<size_t>(fragHandle));

		fragCtx->BaseResourceHandle = fsHandle;
		fragCtx->Size = fsSize;

		Shader_OpenGL* shader = new Shader_OpenGL(vertHandle);
		shader->Handle = vertHandle;
		shader->VertexHandle = vertHandle;
		shader->FragmentHandle = fragHandle;

		shader->Backend = this;

		LogInfo(LogName, "Created a new shader program (vert: %d, frag: %d)", vertHandle, fragHandle);
		return shader;
	}

	Texture* OpenGLBackend::CreateTexture(u32 width, u32 height, TextureFormat format, bool nearestFilter, bool clamp)
	{
		if (width == 0 || height == 0)
		{
			return nullptr;
		}

		if (!MathExtensions::IsPowerOf2(width) || !MathExtensions::IsPowerOf2(height))
		{
			return nullptr;
		}

		GLuint texHandle = 0;
		glGenTextures(1, &texHandle);
		glBindTexture(GL_TEXTURE_2D, texHandle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nearestFilter ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nearestFilter ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);

		GLenum pixelFormat = ConversionTables::GLTextureDisplayFormats[static_cast<size_t>(format)];
		GLenum dataFormat = ConversionTables::GLTextureDataFormats[static_cast<size_t>(format)];
		glTexImage2D(GL_TEXTURE_2D, 0, dataFormat, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, NULL);

		ResourceHandle resHandle = impl->FindFreeResourceContext();
		ResourceContext* resCtx = &impl->ResourceContexts.at(static_cast<size_t>(resHandle));

		resCtx->BaseResourceHandle = texHandle;
		resCtx->Size = static_cast<size_t>(width * height) * TextureFormatPixelSizes[static_cast<size_t>(format)];

		Texture_OpenGL* texture = new Texture_OpenGL(resHandle, width, height, format, clamp, nearestFilter);
		texture->Backend = this;

		LogInfo(LogName, "Created a new texture with handle %d", resHandle);
		return texture;
	}

	void OpenGLBackend::DeleteResource(Resource* resource)
	{
		if (resource->Handle == InvalidResourceHandle)
		{
			if (resource->Type == ResourceType::VertexDesc)
			{
				VertexDesc_OpenGL* desc = static_cast<VertexDesc_OpenGL*>(resource);
				desc->GLAttribs.clear();

				delete desc;
				LogInfo(LogName, "Vertex description has been deleted");
				return;
			}
			return;
		}

		ResourceHandle handle = resource->Handle;
		ResourceContext* resourceCtx = &impl->ResourceContexts.at(static_cast<size_t>(handle));

		switch (resource->Type)
		{
		case ResourceType::VertexBuffer:
		{
			glDeleteBuffersARB(1, &resourceCtx->BaseResourceHandle);
			resourceCtx->BaseResourceHandle = 0;
			resourceCtx->Size = 0;

			VertexBuffer_OpenGL* buffer = static_cast<VertexBuffer_OpenGL*>(resource);
			delete buffer;

			LogInfo(LogName, "Vertex buffer with handle %d has been deleted", handle);
			break;
		}
		case ResourceType::IndexBuffer:
		{
			glDeleteBuffersARB(1, &resourceCtx->BaseResourceHandle);
			resourceCtx->BaseResourceHandle = 0;
			resourceCtx->Size = 0;

			IndexBuffer_OpenGL* buffer = static_cast<IndexBuffer_OpenGL*>(resource);
			delete buffer;

			LogInfo(LogName, "Index buffer with handle %d has been deleted", handle);
			break;
		}
		case ResourceType::Shader:
		{
			Shader_OpenGL* shader = static_cast<Shader_OpenGL*>(resource);

			ResourceContext* vertexCtx = &impl->ResourceContexts.at(static_cast<size_t>(shader->VertexHandle));
			ResourceContext* fragmentCtx = &impl->ResourceContexts.at(static_cast<size_t>(shader->FragmentHandle));

			glDeleteProgramsARB(1, &vertexCtx->BaseResourceHandle);
			glDeleteProgramsARB(1, &fragmentCtx->BaseResourceHandle);

			vertexCtx->BaseResourceHandle = 0;
			vertexCtx->Size = 0;

			fragmentCtx->BaseResourceHandle = 0;
			fragmentCtx->Size = 0;

			ResourceHandle vsHandle = shader->VertexHandle;
			ResourceHandle fsHandle = shader->FragmentHandle;
			delete shader;

			LogInfo(LogName, "Shader program (vert: %d, frag: %d) has been deleted", vsHandle, fsHandle);
			break;
		}
		case ResourceType::Texture:
		{
			glDeleteTextures(1, &resourceCtx->BaseResourceHandle);
			resourceCtx->BaseResourceHandle = 0;
			resourceCtx->Size = 0;

			Texture_OpenGL* texture = static_cast<Texture_OpenGL*>(resource);
			delete texture;

			LogInfo(LogName, "Texture with handle %d has been deleted", handle);
			break;
		}
		}
	}

	void OpenGLBackend::SetVertexBuffer(const VertexBuffer* buffer)
	{
		if (buffer == nullptr)
		{
			impl->SetVertexBuffer(nullptr);
		}
		else
		{
			const VertexBuffer_OpenGL* glBuffer = static_cast<const VertexBuffer_OpenGL*>(buffer);
			impl->SetVertexBuffer(glBuffer);
		}
	}

	void OpenGLBackend::SetIndexBuffer(const IndexBuffer* buffer)
	{
		if (buffer == nullptr)
		{
			impl->SetIndexBuffer(nullptr);
		}
		else
		{
			const IndexBuffer_OpenGL* glBuffer = static_cast<const IndexBuffer_OpenGL*>(buffer);
			impl->SetIndexBuffer(glBuffer);
		}
	}

	void OpenGLBackend::SetVertexDesc(const VertexDesc* desc)
	{
		if (desc != nullptr)
		{
			const VertexDesc_OpenGL* glDesc = static_cast<const VertexDesc_OpenGL*>(desc);
			impl->SetVertexDesc(glDesc);
		}
	}

	void OpenGLBackend::SetShader(Shader* shader)
	{
		if (shader == nullptr)
		{
			impl->SetShader(nullptr);
		}
		else
		{
			Shader_OpenGL* glShader = static_cast<Shader_OpenGL*>(shader);
			impl->SetShader(glShader);
		}
	}

	void OpenGLBackend::SetTexture(const Texture* texture, u32 slot)
	{
		if (texture == nullptr)
		{
			impl->SetTexture(nullptr, slot);
		}
		else
		{
			const Texture_OpenGL* glTexture = static_cast<const Texture_OpenGL*>(texture);
			impl->SetTexture(glTexture, slot);
		}
	}

	void SetBufferData(GLuint glHandle, ResourceType bufferType, void* source, size_t offset, size_t size)
	{
		GLenum bindTarget = 0;

		switch (bufferType)
		{
		case ResourceType::VertexBuffer:
			bindTarget = GL_ARRAY_BUFFER_ARB;
			break;
		case ResourceType::IndexBuffer:
			bindTarget = GL_ELEMENT_ARRAY_BUFFER_ARB;
			break;
		}

		GLintptr copyOffset = static_cast<GLintptr>(offset);
		GLsizeiptr copySize = static_cast<GLsizeiptr>(size);

		glBindBufferARB(bindTarget, glHandle);
		glBufferSubDataARB(bindTarget, copyOffset, copySize, source);
	}

	void VertexBuffer_OpenGL::SetData(void* source, size_t offset, size_t size)
	{
		if (Handle == InvalidResourceHandle)
		{
			return;
		}

		if (offset + size > Size || !Dynamic)
		{
			return;
		}

		ResourceContext* ctx = Backend->GetResourceContext(Handle);
		SetBufferData(ctx->BaseResourceHandle, Type, source, offset, size);
	}

	void IndexBuffer_OpenGL::SetData(void* source, size_t offset, size_t size)
	{
		if (Handle == InvalidResourceHandle)
		{
			return;
		}

		if (offset + size > Size || !Dynamic)
		{
			return;
		}

		ResourceContext* ctx = Backend->GetResourceContext(Handle);
		SetBufferData(ctx->BaseResourceHandle, Type, source, offset, size);
	}

	void Shader_OpenGL::AddVariable(ShaderVariable& variable)
	{
		Variables.push_back(variable);
	}

	ShaderVariableIndex Shader_OpenGL::GetVariableIndex(std::string_view name)
	{
		for (size_t i = 0; i < Variables.size(); i++)
		{
			if (Variables.at(i).Name == name)
			{
				return static_cast<ShaderVariableIndex>(i);
			}
		}

		return InvalidShaderVariable;
	}

	void Shader_OpenGL::SetVariableValue(ShaderVariableIndex varIndex, void* value)
	{
		if (varIndex == InvalidShaderVariable)
		{
			return;
		}

		ShaderVariable& varToUpdate = Variables.at(static_cast<size_t>(varIndex));
		varToUpdate.Value = value;

		UpdateVariables = true;
	}

	u32 Texture_OpenGL::GetWidth() const
	{
		return Width;
	}

	u32 Texture_OpenGL::GetHeight() const
	{
		return Height;
	}

	void Texture_OpenGL::SetData(u32 x, u32 y, u32 width, u32 height, const void* data)
	{
		if (Handle == InvalidResourceHandle)
		{
			return;
		}

		if (x + width > Width || y + height > Height)
		{
			return;
		}

		ResourceContext* ctx = Backend->GetResourceContext(Handle);
		GLenum pixelFormat = ConversionTables::GLTextureDisplayFormats[static_cast<size_t>(Format)];

		glBindTexture(GL_TEXTURE_2D, ctx->BaseResourceHandle);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, pixelFormat, GL_UNSIGNED_BYTE, data);
	}
}
