#ifdef _WIN32
#ifdef STARSHINE_GFX_D3D9
#include <d3d9.h>
#include "d3d9_backend.h"
#include "d3d9_buffers.h"
#include "d3d9_shader.h"
#include "d3d9_vertex_desc.h"
#include "d3d9_texture.h"
#include "SDL2/SDL_syswm.h"
#include "../../../util/logging.h"

namespace GFX
{
	namespace LowLevel
	{
		namespace GFXtoD3D
		{
			/*GLenum BufferType[] =
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
				1,	// INDEX_8BIT
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
				GL_UNSIGNED_BYTE,	// INDEX_8BIT
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
			};*/

			D3DPRIMITIVETYPE PrimitiveTypes[] =
			{
				D3DPT_POINTLIST,		// PRIMITIVE_POINTS

				D3DPT_LINELIST,			// PRIMITIVE_LINES
				D3DPT_LINESTRIP,		// PRIMITIVE_LINE_STRIP

				D3DPT_TRIANGLELIST,		// PRIMITIVE_TRIANGLES
				D3DPT_TRIANGLESTRIP,	// PRIMITIVE_TRIANGLE_STRIP
				D3DPT_TRIANGLEFAN		// PRIMITIVE_TRIANGLE_FAN
			};

			UINT VertexCountPerPrimitive[] = 
			{
				1,		// PRIMITIVE_POINTS

				2,		// PRIMITIVE_LINES
				2,		// PRIMITIVE_LINE_STRIP

				3,		// PRIMITIVE_TRIANGLES
				3,		// PRIMITIVE_TRIANGLE_STRIP
				3		// PRIMITIVE_TRIANGLE_FAN
			};
		}

		namespace D3D9
		{
			/* Implementation */

			bool Backend_D3D9::Initialize(SDL_Window* window)
			{
				if (initialized)
				{
					Logging::LogWarn("GFX::D3D9", "Graphics backend already has been initialized.");
					return true;
				}

				this->targetWindow = window;

				HRESULT result = 0;
				d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

				if (d3d9 == NULL)
				{
					return false;
				}

				D3DPRESENT_PARAMETERS presentParams = {};
				presentParams.Windowed = TRUE;
				presentParams.SwapEffect = D3DSWAPEFFECT::D3DSWAPEFFECT_FLIP;

				SDL_SysWMinfo wmInfo = {};
				SDL_GetWindowWMInfo(targetWindow, &wmInfo);

				if ((result = d3d9->CreateDevice(0, D3DDEVTYPE_HAL, wmInfo.info.win.window, 
												D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams, &d3d9Device)) != S_OK)
				{
					return false;
				}

				int width = 0;
				int height = 0;
				SDL_GetWindowSizeInPixels(targetWindow, &width, &height);

				d3d9Device->BeginScene();
				ResizeMainRenderTarget(width, height);
				d3d9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				d3d9Device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
				d3d9Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
				d3d9Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
				d3d9Device->SetRenderState(D3DRS_LASTPIXEL, FALSE);
				d3d9Device->SetRenderState(D3DRS_CLIPPING, FALSE);
				d3d9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
				d3d9Device->EndScene();

				initialized = true;
				return true;
			}

			void Backend_D3D9::Destroy()
			{
				if (!initialized)
				{
					return;
				}

				d3d9Device->Release();
				d3d9->Release();
				initialized = false;
			}

			void Backend_D3D9::Clear(u32 flags, Color color, float depth, u8 stencil)
			{
				DWORD clearFlags = 0;
				D3DCOLOR clearColor = 0;

				if ((flags & ClearFlags::GFX_CLEAR_COLOR) == ClearFlags::GFX_CLEAR_COLOR)
				{
					clearColor = D3DCOLOR_ARGB(color.A, color.R, color.G, color.B);
					clearFlags |= D3DCLEAR_TARGET;
				}

				if ((flags & ClearFlags::GFX_CLEAR_DEPTH) == ClearFlags::GFX_CLEAR_DEPTH)
				{
					clearFlags |= D3DCLEAR_ZBUFFER;
				}

				if ((flags & ClearFlags::GFX_CLEAR_STENCIL) == ClearFlags::GFX_CLEAR_STENCIL)
				{
					clearFlags |= D3DCLEAR_STENCIL;
				}

				d3d9Device->BeginScene();
				d3d9Device->Clear(0, NULL, clearFlags, clearColor, depth, stencil);
			}

			void Backend_D3D9::SwapBuffers()
			{
				d3d9Device->EndScene();
				d3d9Device->Present(NULL, NULL, NULL, NULL);
			}
			
			void Backend_D3D9::Begin()
			{
				d3d9Device->BeginScene();
			}
			
			void Backend_D3D9::End()
			{
				d3d9Device->EndScene();
			}
			
			void Backend_D3D9::GetViewportSize(float* x, float* y, float* width, float* height)
			{
				D3DVIEWPORT9 viewport = {};
				d3d9Device->GetViewport(&viewport);

				*x = viewport.X;
				*y = viewport.Y;
				*width = viewport.Width;
				*height = viewport.Height;
			}

			void Backend_D3D9::ResizeMainRenderTarget(u32 newWidth, u32 newHeight)
			{
				D3DVIEWPORT9 viewport = {};
				viewport.X = 0;
				viewport.Y = 0;
				viewport.Width = newWidth;
				viewport.Height = newHeight;
				viewport.MinZ = 0.0f;
				viewport.MaxZ = 1.0f;

				d3d9Device->SetViewport(&viewport);

				Logging::LogInfo("GFX::D3D9", "New main render target size: %dx%d\n", newWidth, newHeight);
			}
			
			void Backend_D3D9::SetBlendState(const BlendState* state)
			{
				/*if (state == nullptr)
				{
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
					static_cast<float>(state->constantColor.A) / 255.0f);*/
			}

			Buffer* Backend_D3D9::CreateVertexBuffer(BufferUsage usage, const void* initialData, u32 size)
			{
				if (usage == BufferUsage::BUFFER_USAGE_STATIC && initialData == nullptr)
				{
					Logging::LogError("GFX::D3D9", "Initial data must be set for a static buffer.\n");
					return nullptr;
				}

				if (size == 0)
				{
					Logging::LogError("GFX::D3D9", "Buffer size must be greater than 0.\n");
					return nullptr;
				}

				Buffer_D3D9* buffer = new Buffer_D3D9;
				
				if (buffer->Initialize(d3d9Device, BufferType::BUFFER_VERTEX, usage, size, initialData) == false)
				{
					delete buffer;
					return nullptr;
				}

				IDirect3DVertexBuffer9* newBuffer = buffer->GetBaseVertexBuffer();
				Logging::LogInfo("GFX::D3D9", "Created a new vertex buffer (pointer: 0x%X)\n", newBuffer);

				return buffer;
			}

			Buffer* Backend_D3D9::CreateIndexBuffer(BufferUsage usage, IndexFormat format, const void* initialData, u32 size)
			{
				if (usage == BufferUsage::BUFFER_USAGE_STATIC && initialData == nullptr)
				{
					Logging::LogError("GFX::D3D9", "Initial data must be set for a static buffer.\n");
					return nullptr;
				}

				if (size == 0)
				{
					Logging::LogError("GFX::D3D9", "Buffer size must be greater than 0.\n");
					return nullptr;
				}

				Buffer_D3D9* buffer = new Buffer_D3D9;
				
				if (buffer->Initialize(d3d9Device, BufferType::BUFFER_INDEX, usage, format, size, initialData) == false)
				{
					delete buffer;
					return nullptr;
				}

				IDirect3DIndexBuffer9* newBuffer = buffer->GetBaseIndexBuffer();
				Logging::LogInfo("GFX::D3D9", "Created a new index buffer (pointer: 0x%X)\n", newBuffer);

				return buffer;
			}

			void Backend_D3D9::DestroyBuffer(Buffer* buffer)
			{
				if (buffer == nullptr)
				{
					return;
				}

				Buffer_D3D9* buffer_d3d = static_cast<Buffer_D3D9*>(buffer);
				buffer_d3d->Destroy();

				Logging::LogInfo("GFX::D3D9", "Destroyed a buffer\n");

				delete buffer;
			}

			void Backend_D3D9::BindVertexBuffer(const Buffer* buffer)
			{
				if (buffer == nullptr)
				{
					d3d9Device->SetStreamSource(0, NULL, 0, 0);
					vertexBufferSet = false;
					return;
				}

				const Buffer_D3D9* buffer_d3d = static_cast<const Buffer_D3D9*>(buffer);
				if (buffer_d3d->GetType() != BufferType::BUFFER_VERTEX)
				{
					return;
				}

				IDirect3DVertexBuffer9* baseBuffer = buffer_d3d->GetBaseVertexBuffer();
				d3d9Device->SetStreamSource(0, baseBuffer, 0, currentVertexStride);
				vertexBufferSet = true;
			}

			void Backend_D3D9::BindIndexBuffer(const Buffer* buffer)
			{
				if (buffer == nullptr)
				{
					d3d9Device->SetIndices(NULL);
					indexBufferSet = false;
					return;
				}

				const Buffer_D3D9* buffer_d3d = static_cast<const Buffer_D3D9*>(buffer);
				if (buffer_d3d->GetType() != BufferType::BUFFER_INDEX)
				{
					return;
				}

				IDirect3DIndexBuffer9* baseBuffer = buffer_d3d->GetBaseIndexBuffer();
				d3d9Device->SetIndices(baseBuffer);

				indexBufferSet = true;
				currentIndexFormat = buffer_d3d->GetIndexFormat();
			}
			
			void Backend_D3D9::SetBufferData(Buffer* buffer, const void* src, u32 offset, u32 size)
			{
				if (buffer == nullptr)
				{
					return;
				}

				Buffer_D3D9* buffer_d3d = static_cast<Buffer_D3D9*>(buffer);
				buffer_d3d->SetData(src, offset, size);
			}

			void* Backend_D3D9::MapBuffer(Buffer* buffer, BufferMapping mapMode)
			{
				/*if (buffer == nullptr)
				{
					return nullptr;
				}

				Buffer_OpenGL* buffer_gl = static_cast<Buffer_OpenGL*>(buffer);
				GLuint handle = buffer_gl->GetHandle();
				GLenum binding = GFXtoGL::BufferType[static_cast<int>(buffer_gl->GetType())];

				if (buffer_gl->Mapped)
				{
					Logging::Log(SDL_LOG_CATEGORY_APPLICATION, "[GFX::OpenGL] Buffer with handle %u has already been mapped\n", handle);
					glBindBufferARB(binding, buffer_gl->GetHandle());

					void* mapPtr = nullptr;
					glGetBufferPointervARB(binding, GL_BUFFER_MAP_POINTER_ARB, &mapPtr);

					return mapPtr;
				}
				
				GLbitfield mapMode = 0;
				if ((mappingFlags & BufferMapping::GFX_BUFFER_MAPPING_WRITE) == BufferMapping::GFX_BUFFER_MAPPING_WRITE)
				{
					if (buffer_gl->GetUsage() == BufferUsage::BUFFER_USAGE_STATIC)
					{
						Logging::Log(SDL_LOG_CATEGORY_APPLICATION, "[GFX::OpenGL] Buffer with handle %u is static and cannot be written to\n", handle);
					}
					else
					{
						mapMode |= GL_WRITE_ONLY_ARB;
					}
				}

				glBindBufferARB(binding, buffer_gl->GetHandle());
				void* mapPtr = glMapBufferARB(binding, mapMode);

				buffer_gl->Mapped = true;
				return mapPtr;*/
				return nullptr;
			}

			void Backend_D3D9::UnmapBuffer(Buffer* buffer)
			{
				/*if (buffer == nullptr)
				{
					return;
				}

				Buffer_OpenGL* buffer_gl = static_cast<Buffer_OpenGL*>(buffer);

				if (buffer_gl->Mapped == false)
				{
					return;
				}

				GLuint handle = buffer_gl->GetHandle();
				GLenum binding = GFXtoGL::BufferType[static_cast<int>(buffer_gl->GetType())];

				glBindBufferARB(binding, buffer_gl->GetHandle());
				glUnmapBufferARB(binding);

				buffer_gl->Mapped = false;*/
			}

			Shader* Backend_D3D9::CreateShader(const u8* vsSource, u32 vsSourceLength, const u8* fsSource, u32 fsSourceLength)
			{
				if (vsSource == nullptr || vsSourceLength == 0 ||
					fsSource == nullptr || fsSourceLength == 0)
				{
					return nullptr;
				}

				Shader_D3D9* shader_d3d = new Shader_D3D9;
				if (!shader_d3d->Initialize(d3d9Device, vsSource, vsSourceLength, fsSource, fsSourceLength))
				{
					delete shader_d3d;
					return nullptr;
				}

				return shader_d3d;
			}

			void Backend_D3D9::DestroyShader(Shader* shader)
			{
				if (shader == nullptr)
				{
					return;
				}

				Shader_D3D9* shader_d3d = static_cast<Shader_D3D9*>(shader);
				shader_d3d->Destroy();

				delete shader;
			}

			void Backend_D3D9::BindShader(const Shader* shader)
			{
				if (shader == nullptr)
				{
					d3d9Device->SetVertexShader(NULL);
					d3d9Device->SetPixelShader(NULL);
					return;
				}

				const Shader_D3D9* shader_d3d = static_cast<const Shader_D3D9*>(shader);
				shader_d3d->Bind();
			}
			
			void Backend_D3D9::SetShaderMatrix(u32 index, const float* matrix)
			{
				d3d9Device->SetVertexShaderConstantF(index, matrix, 4);
			}

			VertexDescription* Backend_D3D9::CreateVertexDescription(const VertexAttribute* attribs, u32 attribCount, u32 stride, const Shader* shader)
			{
				if (attribs == nullptr || attribCount == 0 || shader == nullptr)
				{
					return nullptr;
				}

				VertexDescription_D3D9* desc = new VertexDescription_D3D9;

				if (!desc->Initialize(d3d9Device, attribs, attribCount, stride))
				{
					delete desc;
					return nullptr;
				}

				return desc;
			}

			void Backend_D3D9::DestroyVertexDescription(VertexDescription* desc)
			{
				if (desc == nullptr)
				{
					return;
				}

				VertexDescription_D3D9* desc_d3d = static_cast<VertexDescription_D3D9*>(desc);
				desc_d3d->Destroy();

				delete desc;
			}

			void Backend_D3D9::SetVertexDescription(const VertexDescription* desc)
			{
				if (desc == nullptr)
				{
					vertexDescSet = false;
					currentVertexStride = 0;
					return;
				}

				const VertexDescription_D3D9* desc_d3d = static_cast<const VertexDescription_D3D9*>(desc);
				desc_d3d->Set();

				vertexDescSet = true;
				currentVertexStride = desc_d3d->GetVertexStride();
			}

			Texture* Backend_D3D9::CreateTexture(u32 width, u32 height, TextureFormat format, u32 flags)
			{
				if (width == 0 || height == 0)
				{
					return nullptr;
				}

				Texture_D3D9* tex_d3d = new Texture_D3D9;

				if (!tex_d3d->Initialize(d3d9Device, width, height, format, flags))
				{
					delete tex_d3d;
					return nullptr;
				}

				return tex_d3d;
			}

			void Backend_D3D9::DestroyTexture(Texture* texture)
			{
				if (texture != nullptr)
				{
					Texture_D3D9* tex_d3d = static_cast<Texture_D3D9*>(texture);
					tex_d3d->Destroy();
					delete texture;
				}
			}

			void Backend_D3D9::BindTexture(const Texture* texture, u32 unit)
			{
				if (texture != nullptr)
				{
					const Texture_D3D9* tex_d3d = static_cast<const Texture_D3D9*>(texture);
					tex_d3d->Bind(unit);
				}
			}

			void Backend_D3D9::SetTextureData(Texture* texture, const void* data)
			{
				if (texture != nullptr && data != nullptr)
				{
					Texture_D3D9* tex_d3d = static_cast<Texture_D3D9*>(texture);
					tex_d3d->SetData(data);
				}
			}

			void Backend_D3D9::SetTextureData(Texture* texture, const void* data, u32 x, u32 y, u32 width, u32 height)
			{
				if (texture != nullptr && data != nullptr)
				{
					Texture_D3D9* tex_d3d = static_cast<Texture_D3D9*>(texture);
					tex_d3d->SetData(data, x, y, width, height);
				}
			}

			void Backend_D3D9::DrawArrays(PrimitiveType type, i32 firstVertex, i32 vertexCount)
			{
				if (vertexBufferSet && vertexDescSet)
				{
					D3DPRIMITIVETYPE primType = GFXtoD3D::PrimitiveTypes[static_cast<int>(type)];
					UINT vtxCountPerPrim =  GFXtoD3D::VertexCountPerPrimitive[static_cast<int>(type)];
					d3d9Device->DrawPrimitive(primType, 0, vertexCount / vtxCountPerPrim);
				}
			}

			void Backend_D3D9::DrawIndexed(PrimitiveType type, i32 vertexCount, i32 firstIndex, i32 indexCount)
			{
				if (vertexBufferSet && vertexDescSet && indexBufferSet)
				{
					D3DPRIMITIVETYPE primType = GFXtoD3D::PrimitiveTypes[static_cast<int>(type)];
					UINT vtxCountPerPrim =  GFXtoD3D::VertexCountPerPrimitive[static_cast<int>(type)];
					d3d9Device->DrawIndexedPrimitive(primType, 0, 0, indexCount, firstIndex, indexCount / vtxCountPerPrim);
				}
			}
		}
	}
}
#endif
#endif
