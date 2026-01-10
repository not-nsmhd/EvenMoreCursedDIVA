#if 0
#include <d3d9.h>
#include <SDL2/SDL_syswm.h>
#include "D3D9Backend.h"
#include <array>
#include <vector>
#include "io/Xml.h"
#include "io/File.h"
#include "util/logging.h"

namespace Starshine::GFX::Core::D3D9
{
	using namespace Logging;
	using namespace Common;
	using std::array;
	using std::vector;
	using std::string_view;

	constexpr const char* LogName = "Starshine::GFX::Core::D3D9";

	struct ResourceContext
	{
		u8* Data = 0;
		size_t Size = 0;
	};

	struct VertexBuffer_D3D9 : public VertexBuffer
	{
	public:
		VertexBuffer_D3D9(ResourceHandle handle, size_t size, bool dynamic)
			: VertexBuffer(handle), Size(size), Dynamic(dynamic) {}

		IDirect3DDevice9* Device = nullptr;
		IDirect3DVertexBuffer9* BaseBuffer = nullptr;

		size_t Size = 0;
		bool Dynamic = false;

		void SetData(void* source, size_t offset, size_t size)
		{
			if (Dynamic)
			{
				void* bufferData = nullptr;

				BaseBuffer->Lock(static_cast<UINT>(offset), static_cast<UINT>(size), &bufferData, D3DLOCK_DISCARD);
				SDL_memcpy(bufferData, source, size);
				BaseBuffer->Unlock();
			}
		}
	};

	struct D3D9Backend::Impl
	{
		SDL_Window* GameWindow = nullptr;

		IDirect3D9* d3d9 = nullptr;
		IDirect3DDevice9* device = nullptr;
		D3DPRESENT_PARAMETERS presentParams = {};

		vector<ResourceContext> ResourceContexts;

		bool VertexBufferSet = false;
		bool VertexDescSet = false;

		bool IndexBufferSet = false;
		//GLenum CurrentIndexFormat = 0;

		bool ShaderSet = false;

		bool Initialize(SDL_Window* gameWindow)
		{
			d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

			presentParams.Windowed = TRUE;
			presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;

			SDL_SysWMinfo wmInfo = {};
			SDL_GetWindowWMInfo(gameWindow, &wmInfo);

			d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wmInfo.info.win.window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams, &device);

			return true;
		}

		void Destroy()
		{
			device->Release();
			d3d9->Release();
		}

		void Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil)
		{
			DWORD clearFlags = 0;
			D3DCOLOR clearColor = 0;

			if ((flags & ClearFlags::ClearFlags_Color) != 0)
			{
				clearColor = D3DCOLOR_ARGB(color.A, color.R, color.G, color.B);
				clearFlags |= D3DCLEAR_TARGET;
			}

			if ((flags & ClearFlags::ClearFlags_Depth) != 0)
			{
				clearFlags |= D3DCLEAR_ZBUFFER;
			}

			if ((flags & ClearFlags::ClearFlags_Stencil) != 0)
			{
				clearFlags |= D3DCLEAR_STENCIL;
			}

			device->Clear(0, NULL, clearFlags, clearColor, depth, stencil);
		}

		void SwapBuffers()
		{
			device->Present(NULL, NULL, NULL, NULL);
		}

		ResourceHandle FindFreeResourceContext()
		{
			for (size_t i = 0; i < ResourceContexts.size(); i++)
			{
				ResourceContext& ctx = ResourceContexts[i];

				if (ctx.Data == nullptr && ctx.Size == 0)
				{
					return i;
				}
			}

			ResourceContexts.emplace_back();
			return static_cast<ResourceHandle>(ResourceContexts.size() - 1);
		}

		VertexBuffer_D3D9* CreateVertexBuffer(size_t size, void* initialData, bool dynamic)
		{
			if (!dynamic && initialData == nullptr || size == 0)
			{
				return nullptr;
			}

			UINT length = static_cast<UINT>(size);
			DWORD usage = dynamic ? D3DUSAGE_DYNAMIC : D3DUSAGE_WRITEONLY;

			ResourceHandle handle = FindFreeResourceContext();
			ResourceContext* ctx = &ResourceContexts.at(static_cast<size_t>(handle));

			ctx->Data = new u8[size];
			ctx->Size = size;

			VertexBuffer_D3D9* buffer = new VertexBuffer_D3D9(handle, size, dynamic);

			buffer->Device = device;
			device->CreateVertexBuffer(length, usage, 0, D3DPOOL_DEFAULT, &buffer->BaseBuffer, NULL);

			if (initialData != nullptr)
			{
				buffer->SetData(initialData, 0, size);
			}

			LogInfo(LogName, "Created a new vertex buffer (handle %d)", handle);
			return buffer;
		}

		/*void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
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
		}*/
	};

	D3D9Backend::D3D9Backend() : impl(new D3D9Backend::Impl())
	{
	}

	D3D9Backend::~D3D9Backend()
	{
	}

	bool D3D9Backend::Initialize(SDL_Window* gameWindow)
	{
		return impl->Initialize(gameWindow);
	}

	void D3D9Backend::Destroy()
	{
		impl->Destroy();
		delete impl;
	}

	RendererBackendType D3D9Backend::GetType() const
	{
		return RendererBackendType::OpenGL;
	}

	ResourceContext* D3D9Backend::GetResourceContext(ResourceHandle handle)
	{
		/*if (handle != InvalidResourceHandle)
		{
			return &impl->ResourceContexts.at(static_cast<size_t>(handle));
		}*/
		return nullptr;
	}

	Common::RectangleF D3D9Backend::GetViewportSize() const
	{
		RectangleF viewport = {};
		//glGetFloatv(GL_VIEWPORT, &viewport.X);
		return viewport;
	}

	void D3D9Backend::Clear(ClearFlags flags, Common::Color& color, f32 depth, u8 stencil)
	{
		impl->Clear(flags, color, depth, stencil);
	}

	void D3D9Backend::SwapBuffers()
	{
		impl->SwapBuffers();
	}

	void D3D9Backend::SetBlendState(bool enable, BlendFactor srcColor, BlendFactor destColor, BlendFactor srcAlpha, BlendFactor destAlpha)
	{
	}

	void D3D9Backend::SetBlendOperation(BlendOperation op)
	{
	}

	void D3D9Backend::DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
	{
		//impl->DrawArrays(type, firstVertex, vertexCount);
	}

	void D3D9Backend::DrawIndexed(PrimitiveType type, u32 firstIndex, u32 indexCount)
	{
		//impl->DrawIndexed(type, firstIndex, indexCount);
	}

	VertexBuffer* D3D9Backend::CreateVertexBuffer(size_t size, void* initialData, bool dynamic)
	{
		return impl->CreateVertexBuffer(size, initialData, dynamic);
	}

	// TODO: Make this less copy-pasty
	IndexBuffer* D3D9Backend::CreateIndexBuffer(size_t size, IndexFormat format, void* initialData, bool dynamic)
	{
		return nullptr;
	}

	VertexDesc* D3D9Backend::CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount)
	{
		return nullptr;
	}

	Shader* D3D9Backend::LoadShader(const u8* vsData, size_t vsSize, const u8* fsData, size_t fsSize)
	{
		return nullptr;
	}

	Texture* D3D9Backend::CreateTexture(u32 width, u32 height, TextureFormat format, bool nearestFilter, bool clamp)
	{
		return nullptr;
	}

	void D3D9Backend::DeleteResource(Resource* resource)
	{
	}

	void D3D9Backend::SetVertexBuffer(const VertexBuffer* buffer)
	{
	}

	void D3D9Backend::SetIndexBuffer(const IndexBuffer* buffer)
	{
	}

	void D3D9Backend::SetVertexDesc(const VertexDesc* desc)
	{
	}

	void D3D9Backend::SetShader(Shader* shader)
	{
	}

	void D3D9Backend::SetTexture(const Texture* texture, u32 slot)
	{
	}
}
#endif
