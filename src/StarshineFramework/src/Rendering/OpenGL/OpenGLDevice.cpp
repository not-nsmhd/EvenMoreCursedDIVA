#include <glad/glad.h>
#include "Common/MathExt.h"
#include "Common/Logging/Logging.h"
#include "Rendering/Types.h"
#include "OpenGLDevice.h"
#include "OpenGLCommon.h"
#include "OpenGLBuffers.h"
#include "OpenGLVertexDesc.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"
#include <array>
#include "IO/Xml.h"
#include "IO/Path/File.h"

namespace Starshine::Rendering::OpenGL
{
	using std::array;
	using std::string_view;

	constexpr const char* LogName = "Starshine::Rendering::OpenGL";

	struct OpenGLDevice::Impl
	{
		SDL_Window* GameWindow = nullptr;
		SDL_GLContext GLContext{};

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

			SDL_GL_SetSwapInterval(1);

			LogInfo(LogName, "OpenGL Version: %s", glGetString(GL_VERSION));
			LogInfo(LogName, "OpenGL Renderer: %s", glGetString(GL_RENDERER));

			glEnable(GL_VERTEX_PROGRAM_ARB);
			glEnable(GL_FRAGMENT_PROGRAM_ARB);
			
			glEnable(GL_CULL_FACE);
			glFrontFace(GL_CW);
			glCullFace(GL_BACK);

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
				GLuint glBufferHandle = buffer->Handle;
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, glBufferHandle);
				VertexBufferSet = true;
			}
		}

		void SetVertexDesc(const VertexDesc_OpenGL* desc)
		{
			if (desc != nullptr)
			{
				for (auto& attrib : desc->Attribs)
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
			else
			{
				VertexDescSet = false;
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
				GLuint glBufferHandle = buffer->Handle;
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, glBufferHandle);
				CurrentIndexFormat = ConversionTables::GLIndexFormats[static_cast<size_t>(buffer->Format)];
				IndexBufferSet = true;
			}
		}
	};

	OpenGLDevice::OpenGLDevice() : impl(std::make_unique<Impl>())
	{
	}

	OpenGLDevice::~OpenGLDevice()
	{
	}

	bool OpenGLDevice::Initialize(SDL_Window* gameWindow)
	{
		return impl->Initialize(gameWindow);
	}

	void OpenGLDevice::Destroy()
	{
		impl->Destroy();
	}

	RectangleF OpenGLDevice::GetViewportSize() const
	{
		RectangleF viewport = {};
		glGetFloatv(GL_VIEWPORT, &viewport.X);
		return viewport;
	}

	void OpenGLDevice::Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil)
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

	void OpenGLDevice::SwapBuffers()
	{
		impl->SwapBuffers();
	}

	void OpenGLDevice::SetBlendState(bool enable, BlendFactor srcColor, BlendFactor destColor, BlendFactor srcAlpha, BlendFactor destAlpha)
	{
		GLboolean blendEnabled = glIsEnabled(GL_BLEND);
		if (enable)
		{
			if (!blendEnabled) { glEnable(GL_BLEND); }
		}
		else
		{
			if (blendEnabled) { glDisable(GL_BLEND); }
			return;
		}

		GLenum glSrcColor = ConversionTables::GLBlendFactors[static_cast<size_t>(srcColor)];
		GLenum glDstColor = ConversionTables::GLBlendFactors[static_cast<size_t>(destColor)];
		GLenum glSrcAlpha = ConversionTables::GLBlendFactors[static_cast<size_t>(srcAlpha)];
		GLenum glDstAlpha = ConversionTables::GLBlendFactors[static_cast<size_t>(destAlpha)];

		glBlendFuncSeparate(glSrcColor, glDstColor, glSrcAlpha, glDstAlpha);
	}

	void OpenGLDevice::SetBlendOperation(BlendOperation op)
	{
		GLenum glOperation = ConversionTables::GLBlendOperations[static_cast<size_t>(op)];
		glBlendEquation(glOperation);
	}

	void OpenGLDevice::SetFaceCullingState(bool enable, PolygonOrientation backFaceOrientation)
	{
		GLboolean cullingEnabled = glIsEnabled(GL_CULL_FACE);
		if (enable)
		{
			if (!cullingEnabled) { glEnable(GL_CULL_FACE); }
		}
		else
		{
			if (cullingEnabled) { glDisable(GL_CULL_FACE); }
			return;
		}

		GLenum glOrientation = ConversionTables::GLPolygonOrientation[static_cast<size_t>(backFaceOrientation)];

		glFrontFace(glOrientation);
		glCullFace(GL_FRONT); // Oh well...
	}

	void OpenGLDevice::DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
	{
		impl->DrawArrays(type, firstVertex, vertexCount);
	}

	void OpenGLDevice::DrawIndexed(PrimitiveType type, u32 firstIndex, u32 vertexCount, u32 indexCount)
	{
		impl->DrawIndexed(type, firstIndex, indexCount);
	}

	std::unique_ptr<VertexBuffer> OpenGLDevice::CreateVertexBuffer(size_t size, const void* initialData, bool dynamic)
	{
		if (!dynamic && initialData == nullptr || size == 0)
		{
			return nullptr;
		}

		GLsizeiptr bufferLength = static_cast<GLsizeiptr>(size);
		GLenum bufferUsage = (dynamic) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB;

		GLuint bufferHandle = 0;
		glGenBuffersARB(1, &bufferHandle);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferHandle);

		if (initialData != nullptr)
		{
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, bufferLength, initialData, bufferUsage);
		}
		else
		{
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, bufferLength, NULL, bufferUsage);
		}

		std::unique_ptr<VertexBuffer_OpenGL> buffer = std::make_unique<VertexBuffer_OpenGL>(*this, size, dynamic);
		buffer->Handle = bufferHandle;

		return buffer;
	}

	std::unique_ptr<IndexBuffer> OpenGLDevice::CreateIndexBuffer(size_t size, IndexFormat format, const void* initialData, bool dynamic)
	{
		if (!dynamic && initialData == nullptr || size == 0)
		{
			return nullptr;
		}

		GLsizeiptr bufferLength = static_cast<GLsizeiptr>(size);
		GLenum bufferUsage = (dynamic) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB;

		GLuint bufferHandle = 0;
		glGenBuffersARB(1, &bufferHandle);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferHandle);

		if (initialData != nullptr)
		{
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferLength, initialData, bufferUsage);
		}
		else
		{
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferLength, NULL, bufferUsage);
		}

		std::unique_ptr<IndexBuffer_OpenGL> buffer = std::make_unique<IndexBuffer_OpenGL>(*this, size, format, dynamic);
		buffer->Handle = bufferHandle;

		return buffer;
	}

	std::unique_ptr<VertexDesc> OpenGLDevice::CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount)
	{
		if (attribs == nullptr || attribCount == 0)
		{
			return nullptr;
		}

		std::unique_ptr<VertexDesc_OpenGL> desc = std::make_unique<VertexDesc_OpenGL>(*this, attribs, attribCount);
		return desc;
	}

	std::unique_ptr<Shader> OpenGLDevice::LoadShader(const void* vsData, size_t vsSize, const void* fsData, size_t fsSize)
	{
		if (vsData == nullptr || vsSize == 0 || fsData == nullptr || fsSize == 0)
		{
			return nullptr;
		}

		GLuint vpHandle = 0;
		GLuint fpHandle = 0;

		glGenProgramsARB(1, &vpHandle);
		glGenProgramsARB(1, &fpHandle);

		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vpHandle);
		glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, static_cast<GLsizei>(vsSize), vsData);

		GLint errorPos = -1;
		const GLubyte* errorString = nullptr;

		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (errorPos != -1)
		{
			errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LogError(LogName, "Failed to assemble vertex program. Error (pos: %d): %s", errorPos, errorString);

			glDeleteProgramsARB(1, &vpHandle);
			return nullptr;
		}

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fpHandle);
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, static_cast<GLsizei>(fsSize), fsData);

		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (errorPos != -1)
		{
			errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			LogError(LogName, "Failed to assemble fragment program. Error (pos: %d): %s", errorPos, errorString);

			glDeleteProgramsARB(1, &vpHandle);
			glDeleteProgramsARB(1, &fpHandle);
			return nullptr;
		}

		std::unique_ptr<Shader_D3D9> shader = std::make_unique<Shader_D3D9>(*this, vpHandle, fpHandle);
		return shader;
	}

	std::unique_ptr<Texture> OpenGLDevice::CreateTexture(u32 width, u32 height, GFX::TextureFormat format, bool nearestFilter, bool repeat)
	{
		if (width == 0 || height == 0) { return nullptr; }
		if (!MathExtensions::IsPowerOf2(width) || !MathExtensions::IsPowerOf2(height)) { return nullptr; }

		GLuint texHandle = 0;
		GLenum filter = nearestFilter ? GL_NEAREST : GL_LINEAR;
		GLenum wrapMode = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;

		glGenTextures(1, &texHandle);
		glBindTexture(GL_TEXTURE_2D, texHandle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

		GLenum pixelFormat = ConversionTables::GLTextureDisplayFormats[static_cast<size_t>(format)];
		GLenum dataFormat = ConversionTables::GLTextureDataFormats[static_cast<size_t>(format)];
		glTexImage2D(GL_TEXTURE_2D, 0, dataFormat, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, NULL);

		std::unique_ptr<Texture_D3D9> texture = std::make_unique<Texture_D3D9>(*this, width, height, format, false);
		texture->Handle = texHandle;
		texture->Filter = filter;
		texture->WrapMode = wrapMode;

		return texture;
	}

	void OpenGLDevice::SetVertexBuffer(const VertexBuffer* buffer, const VertexDesc* desc)
	{
		if (buffer == nullptr || desc == nullptr)
		{
			impl->SetVertexDesc(nullptr);
			impl->SetVertexBuffer(nullptr);
		}
		else
		{
			const VertexBuffer_OpenGL* glBuffer = static_cast<const VertexBuffer_OpenGL*>(buffer);
			const VertexDesc_OpenGL* glDesc = static_cast<const VertexDesc_OpenGL*>(desc);
			impl->SetVertexDesc(glDesc);
			impl->SetVertexBuffer(glBuffer);
		}
	}

	void OpenGLDevice::SetIndexBuffer(const IndexBuffer* buffer)
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

	void OpenGLDevice::SetShader(const Shader* shader)
	{
		if (shader == nullptr)
		{
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
			impl->ShaderSet = false;
		}
		else
		{
			const Shader_D3D9* glShader = static_cast<const Shader_D3D9*>(shader);
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, glShader->VertexProgramHandle);
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, glShader->FragmentProgramHandle);
			impl->ShaderSet = true;
		}
	}

	void OpenGLDevice::SetTexture(const Texture* texture, u32 slot)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB + slot);
		if (texture == nullptr)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			const Texture_D3D9* glTexture = static_cast<const Texture_D3D9*>(texture);
			glBindTexture(GL_TEXTURE_2D, glTexture->Handle);
		}
	}

	/*void Shader_OpenGL::AddVariable(ShaderVariable& variable)
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
	}*/
}
