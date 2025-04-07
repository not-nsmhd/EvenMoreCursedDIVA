#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "opengl_defs.h"
#include "opengl_buffers.h"
#include "opengl_shader.h"
#include "opengl_vertex_desc.h"
#include "opengl_texture.h"
#include "opengl_backend.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace GFXtoGL
		{
			GLenum BufferType[] =
			{
				GL_ARRAY_BUFFER_ARB,			// BUFFER_VERTEX
				GL_ELEMENT_ARRAY_BUFFER_ARB		// BUFFER_INDEX
			};

			GLenum BufferUsage[] =
			{
				GL_STATIC_DRAW_ARB,		// BUFFER_USAGE_STATIC
				GL_DYNAMIC_DRAW_ARB		// BUFFER_USAGE_DYNAMIC
			};

			size_t IndexSizes[] =
			{
				2,	// INDEX_16BIT
				4	// INDEX_32BIT
			};

			GLenum PrimitiveTypes[] =
			{
				GL_POINTS,			// PRIMITIVE_POINTS

				GL_LINES,			// PRIMITIVE_LINES
				GL_LINE_STRIP,		// PRIMITIVE_LINE_STRIP

				GL_TRIANGLES,		// PRIMITIVE_TRIANGLES
				GL_TRIANGLE_STRIP,	// PRIMITIVE_TRIANGLE_STRIP
				GL_TRIANGLE_FAN		// PRIMITIVE_TRIANGLE_FAN
			};

			GLenum IndexType[] =
			{
				GL_UNSIGNED_SHORT,	// INDEX_16BIT
				GL_UNSIGNED_INT		// INDEX_32BIT
			};

			GLenum BlendFactor[] = 
			{
				GL_ZERO, // BLEND_ZERO
				GL_ONE, // BLEND_ONE

				GL_SRC_COLOR, // BLEND_SRC_COLOR
				GL_ONE_MINUS_SRC_COLOR, // BLEND_ONE_MINUS_SRC_COLOR
				GL_DST_COLOR, // BLEND_DST_COLOR
				GL_ONE_MINUS_DST_COLOR, // BLEND_ONE_MINUS_DST_COLOR
 
				GL_SRC_ALPHA, // BLEND_SRC_ALPHA
				GL_ONE_MINUS_SRC_ALPHA, // BLEND_ONE_MINUS_SRC_ALPHA
				GL_DST_ALPHA, // BLEND_DST_ALPHA
				GL_ONE_MINUS_DST_ALPHA, // BLEND_ONE_MINUS_DST_ALPHA
 
				GL_CONSTANT_COLOR, // BLEND_CONSTANT_COLOR
				GL_ONE_MINUS_CONSTANT_COLOR, // BLEND_ONE_MINUS_CONSTANT_COLOR
 
				GL_CONSTANT_ALPHA, // BLEND_CONSTANT_ALPHA
				GL_ONE_MINUS_CONSTANT_ALPHA // BLEND_ONE_MINUS_CONSTANT_ALPHA
			};

			GLenum BlendOp[] = 
			{
				GL_FUNC_ADD, // BLEND_OP_ADD
				GL_FUNC_SUBTRACT, // BLEND_OP_SUBTRACT
				GL_FUNC_REVERSE_SUBTRACT, // BLEND_OP_REVERSE_SUBTRACT
				GL_MIN, // BLEND_OP_MIN
				GL_MAX // BLEND_OP_MAX
			};
		}

		namespace OpenGL_ARB
		{
			/* Implementation */

			bool Backend_OpenGL::Initialize(SDL_Window* window)
			{
				if (initialized)
				{
					LOG_WARN("Graphics backned has already been initialized");
					return true;
				}

				u32 windowFlags = SDL_GetWindowFlags(window);
				if ((windowFlags & SDL_WINDOW_OPENGL) != SDL_WINDOW_OPENGL)
				{
					LOG_ERROR("Game window is not an OpenGL window");
					return false;
				}

				this->targetWindow = window;
				if ((glContext = SDL_GL_CreateContext(window)) == NULL)
				{
					char message[512] = {};
					SDL_snprintf(message, 511, "Failed to create an OpenGL context.\nError: %s", SDL_GetError());

					LOG_ERROR_ARGS("%s", message);
					SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (GFX)", message, targetWindow);

					return false;
				}

				if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
				{
					LOG_ERROR("Failed to load OpenGL functions");
					SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (GFX)", "Failed to load OpenGL functions", targetWindow);
					SDL_GL_DeleteContext(glContext);
					return false;
				}

				SDL_GL_SetSwapInterval(1);

				LOG_INFO_ARGS("Version: %s", glGetString(GL_VERSION));
				LOG_INFO_ARGS("Renderer: %s", glGetString(GL_RENDERER));

				//glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				glFrontFace(GL_CW);

				glEnable(GL_VERTEX_PROGRAM_ARB);
				glEnable(GL_FRAGMENT_PROGRAM_ARB);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);

				initialized = true;
				return true;
			}

			void Backend_OpenGL::Destroy()
			{
				if (!initialized)
				{
					return;
				}

				SDL_GL_DeleteContext(glContext);
				initialized = false;
			}

			void Backend_OpenGL::Clear(u32 flags, Color color, float depth, u8 stencil)
			{
				GLenum clearFlags = 0;

				if ((flags & ClearFlags::GFX_CLEAR_COLOR) == ClearFlags::GFX_CLEAR_COLOR)
				{
					glClearColor(
						static_cast<float>(color.R) / 255.0f,
						static_cast<float>(color.G) / 255.0f,
						static_cast<float>(color.B) / 255.0f,
						static_cast<float>(color.A) / 255.0f);

					clearFlags |= GL_COLOR_BUFFER_BIT;
				}

				if ((flags & ClearFlags::GFX_CLEAR_DEPTH) == ClearFlags::GFX_CLEAR_DEPTH)
				{
					glClearDepth(static_cast<double>(depth));
					clearFlags |= GL_DEPTH_BUFFER_BIT;
				}

				if ((flags & ClearFlags::GFX_CLEAR_STENCIL) == ClearFlags::GFX_CLEAR_STENCIL)
				{
					glClearStencil(stencil);
					clearFlags |= GL_STENCIL_BUFFER_BIT;
				}

				glClear(clearFlags);
			}

			void Backend_OpenGL::SwapBuffers()
			{
				SDL_GL_SwapWindow(targetWindow);
			}
			
			void Backend_OpenGL::Begin()
			{	
			}
			
			void Backend_OpenGL::End()
			{
			}
			
			void Backend_OpenGL::GetViewportSize(float* x, float* y, float* width, float* height)
			{
				float viewport[4] = {};
				glGetFloatv(GL_VIEWPORT, viewport);

				*x = viewport[0];
				*y = viewport[1];
				*width = viewport[2];
				*height = viewport[3];
			}

			void Backend_OpenGL::ResizeMainRenderTarget(u32 newWidth, u32 newHeight)
			{
				glViewport(0, 0, newWidth, newHeight);
				LOG_INFO_ARGS("New main render target size: %dx%d", newWidth, newHeight);
			}
			
			void Backend_OpenGL::SetBlendState(const BlendState* state)
			{
				if (state == nullptr)
				{
					glDisable(GL_BLEND);
					return;
				}

				if (glIsEnabled(GL_BLEND) != GL_TRUE)
				{
					glEnable(GL_BLEND);
				}

				GLenum srcRGB = GFXtoGL::BlendFactor[static_cast<int>(state->srcColor)];
				GLenum srcAlpha = GFXtoGL::BlendFactor[static_cast<int>(state->srcAlpha)];

				GLenum dstRGB = GFXtoGL::BlendFactor[static_cast<int>(state->dstColor)];
				GLenum dstAlpha = GFXtoGL::BlendFactor[static_cast<int>(state->dstAlpha)];

				GLenum rgbOp = GFXtoGL::BlendOp[static_cast<int>(state->colorOp)];

				glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
				glBlendEquation(rgbOp);

				glBlendColor(
					static_cast<float>(state->constantColor.R) / 255.0f,
					static_cast<float>(state->constantColor.G) / 255.0f,
					static_cast<float>(state->constantColor.B) / 255.0f,
					static_cast<float>(state->constantColor.A) / 255.0f);
			}

			Buffer* Backend_OpenGL::CreateVertexBuffer(BufferUsage usage, const void* initialData, u32 size)
			{
				if (usage == BufferUsage::BUFFER_USAGE_STATIC && initialData == nullptr)
				{
					return nullptr;
				}

				if (size == 0)
				{
					return nullptr;
				}

				Buffer_OpenGL* buffer = new Buffer_OpenGL;
				
				if (buffer->Initialize(BufferType::BUFFER_VERTEX, usage, size, initialData) == false)
				{
					delete buffer;
					return nullptr;
				}

				return buffer;
			}

			Buffer* Backend_OpenGL::CreateIndexBuffer(BufferUsage usage, IndexFormat format, const void* initialData, u32 size)
			{
				if (usage == BufferUsage::BUFFER_USAGE_STATIC && initialData == nullptr)
				{
					return nullptr;
				}

				if (size == 0)
				{
					return nullptr;
				}

				Buffer_OpenGL* buffer = new Buffer_OpenGL;

				if (buffer->Initialize(BufferType::BUFFER_INDEX, usage, format, size, initialData) == false)
				{
					delete buffer;
					return nullptr;
				}

				return buffer;
			}

			void Backend_OpenGL::DestroyBuffer(Buffer* buffer)
			{
				if (buffer == nullptr)
				{
					return;
				}

				Buffer_OpenGL* buffer_gl = static_cast<Buffer_OpenGL*>(buffer);
				buffer_gl->Destroy();

				delete buffer;
			}

			void Backend_OpenGL::BindVertexBuffer(Buffer* buffer)
			{
				if (buffer == nullptr)
				{
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
					return;
				}

				const Buffer_OpenGL* buffer_gl = static_cast<const Buffer_OpenGL*>(buffer);
				if (buffer_gl->GetType() != BufferType::BUFFER_VERTEX)
				{
					return;
				}

				buffer_gl->Bind();
			}

			void Backend_OpenGL::BindIndexBuffer(Buffer* buffer)
			{
				if (buffer == nullptr)
				{
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
					indexBufferSet = false;
					return;
				}

				const Buffer_OpenGL* buffer_gl = static_cast<const Buffer_OpenGL*>(buffer);
				if (buffer_gl->GetType() != BufferType::BUFFER_INDEX)
				{
					return;
				}

				buffer_gl->Bind();
				indexBufferSet = true;
				currentIndexFormat = buffer_gl->GetIndexFormat();
			}
			
			void Backend_OpenGL::SetBufferData(Buffer* buffer, const void* src, u32 offset, u32 size)
			{
				if (buffer == nullptr)
				{
					return;
				}

				Buffer_OpenGL* buffer_gl = static_cast<Buffer_OpenGL*>(buffer);
				buffer_gl->SetData(src, offset, size);
			}

			void* Backend_OpenGL::MapBuffer(Buffer* buffer, BufferMapping mapMode)
			{
				if (buffer == nullptr)
				{
					return nullptr;
				}

				Buffer_OpenGL* buffer_gl = static_cast<Buffer_OpenGL*>(buffer);
				return buffer_gl->Map(mapMode);
			}

			void Backend_OpenGL::UnmapBuffer(Buffer* buffer)
			{
				if (buffer == nullptr)
				{
					return;
				}

				Buffer_OpenGL* buffer_gl = static_cast<Buffer_OpenGL*>(buffer);
				buffer_gl->Unmap();
			}

			Shader* Backend_OpenGL::CreateShader(const u8* vsSource, u32 vsSourceLength, const u8* fsSource, u32 fsSourceLength)
			{
				if (vsSource == nullptr || vsSourceLength == 0 ||
					fsSource == nullptr || fsSourceLength == 0)
				{
					return nullptr;
				}

				Shader_OpenGL* shader_gl = new Shader_OpenGL;
				if (!shader_gl->Initialize(vsSource, vsSourceLength, fsSource, fsSourceLength))
				{
					delete shader_gl;
					return nullptr;
				}

				return shader_gl;
			}

			void Backend_OpenGL::DestroyShader(Shader* shader)
			{
				if (shader == nullptr)
				{
					return;
				}

				Shader_OpenGL* shader_gl = static_cast<Shader_OpenGL*>(shader);
				shader_gl->Destroy();

				delete shader;
			}

			void Backend_OpenGL::BindShader(Shader* shader)
			{
				if (shader == nullptr)
				{
					glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
					glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
					return;
				}

				const Shader_OpenGL* shader_gl = static_cast<const Shader_OpenGL*>(shader);
				shader_gl->Bind();
			}
			
			void Backend_OpenGL::SetShaderMatrix(u32 index, const float* matrix)
			{
				glMatrixMode(GL_MATRIX0_ARB + index);
				if (matrix == nullptr)
				{
					glLoadIdentity();
				}
				else
				{
					glLoadMatrixf(matrix);
				}
			}
			
			void Backend_OpenGL::SetShaderMatrix(u32 index, const mat4* matrix)
			{
				glMatrixMode(GL_MATRIX0_ARB + index);
				if (matrix == nullptr)
				{
					glLoadIdentity();
				}
				else
				{
					glLoadMatrixf(glm::value_ptr(*matrix));
				}
			}

			VertexDescription* Backend_OpenGL::CreateVertexDescription(const VertexAttribute* attribs, u32 attribCount, u32 stride, const Shader* shader)
			{
				if (attribs == nullptr || attribCount == 0 || shader == nullptr)
				{
					return nullptr;
				}

				VertexDescription_OpenGL* desc = new VertexDescription_OpenGL;

				if (!desc->Initialize(attribs, attribCount, stride))
				{
					delete desc;
					return nullptr;
				}

				return desc;
			}

			void Backend_OpenGL::DestroyVertexDescription(VertexDescription* desc)
			{
				if (desc == nullptr)
				{
					return;
				}

				VertexDescription_OpenGL* desc_gl = static_cast<VertexDescription_OpenGL*>(desc);
				desc_gl->Destroy();

				delete desc;
			}

			void Backend_OpenGL::SetVertexDescription(VertexDescription* desc)
			{
				if (desc == nullptr)
				{
					return;
				}

				const VertexDescription_OpenGL* desc_gl = static_cast<const VertexDescription_OpenGL*>(desc);
				desc_gl->Set();
			}

			Texture* Backend_OpenGL::CreateTexture(u32 width, u32 height, TextureFormat format, u32 flags)
			{
				if (width == 0 || height == 0)
				{
					return nullptr;
				}

				Texture_OpenGL* tex_gl = new Texture_OpenGL;

				if (!tex_gl->Initialize(width, height, format, flags))
				{
					delete tex_gl;
					return nullptr;
				}

				return tex_gl;
			}

			void Backend_OpenGL::DestroyTexture(Texture* texture)
			{
				if (texture != nullptr)
				{
					Texture_OpenGL* tex_gl = static_cast<Texture_OpenGL*>(texture);
					tex_gl->Destroy();
					delete texture;
				}
			}

			void Backend_OpenGL::BindTexture(Texture* texture, u32 unit)
			{
				if (texture != nullptr)
				{
					const Texture_OpenGL* tex_gl = static_cast<const Texture_OpenGL*>(texture);
					tex_gl->Bind(unit);
				}
			}

			void Backend_OpenGL::SetTextureData(Texture* texture, const void* data)
			{
				if (texture != nullptr && data != nullptr)
				{
					Texture_OpenGL* tex_gl = static_cast<Texture_OpenGL*>(texture);
					tex_gl->SetData(data);
				}
			}

			void Backend_OpenGL::SetTextureData(Texture* texture, const void* data, u32 x, u32 y, u32 width, u32 height)
			{
				if (texture != nullptr && data != nullptr)
				{
					Texture_OpenGL* tex_gl = static_cast<Texture_OpenGL*>(texture);
					tex_gl->SetData(data, x, y, width, height);
				}
			}

			void Backend_OpenGL::DrawArrays(PrimitiveType type, i32 firstVertex, i32 vertexCount)
			{
				GLenum primType = GFXtoGL::PrimitiveTypes[static_cast<int>(type)];
				glDrawArrays(primType, firstVertex, vertexCount);
			}

			void Backend_OpenGL::DrawIndexed(PrimitiveType type, i32 vertexCount, i32 firstIndex, i32 indexCount)
			{
				if (!indexBufferSet)
				{
					return;
				}

				GLenum primType = GFXtoGL::PrimitiveTypes[static_cast<int>(type)];
				GLenum indexType = GFXtoGL::IndexType[static_cast<int>(currentIndexFormat)];
				size_t indexSize = GFXtoGL::IndexSizes[static_cast<int>(currentIndexFormat)];
				void* firstIndexOffset = (void*)(firstIndex * indexSize);

				glDrawElements(primType, indexCount, indexType, firstIndexOffset);
			}
		}
	}
}
