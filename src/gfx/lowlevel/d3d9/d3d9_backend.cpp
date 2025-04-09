#include <SDL2/SDL_syswm.h>
#include <glm/gtc/type_ptr.hpp>
#include "d3d9_backend.h"
#include "d3d9_buffers.h"
#include "d3d9_shader.h"
#include "d3d9_vertex_desc.h"
#include "d3d9_texture.h"
#include "d3d9_defs.h"

namespace GFX::LowLevel::D3D9
{
	namespace GFXtoD3D9
	{
		D3DPRIMITIVETYPE PrimitiveTypes[] =
		{
			D3DPRIMITIVETYPE::D3DPT_POINTLIST,		// PRIMITIVE_POINTS

			D3DPRIMITIVETYPE::D3DPT_LINELIST,		// PRIMITIVE_LINES
			D3DPRIMITIVETYPE::D3DPT_LINESTRIP,		// PRIMITIVE_LINE_STRIP

			D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST,	// PRIMITIVE_TRIANGLES
			D3DPRIMITIVETYPE::D3DPT_TRIANGLESTRIP,	// PRIMITIVE_TRIANGLE_STRIP
			D3DPRIMITIVETYPE::D3DPT_TRIANGLEFAN		// PRIMITIVE_TRIANGLE_FAN
		};

		UINT VerticesPerPrimitive[] = 
		{
			1,		// PRIMITIVE_POINTS

			2,		// PRIMITIVE_LINES
			2,		// PRIMITIVE_LINE_STRIP

			3,	// PRIMITIVE_TRIANGLES
			3,	// PRIMITIVE_TRIANGLE_STRIP
			3		// PRIMITIVE_TRIANGLE_FAN
		};
	}

	bool Backend_D3D9::Initialize(SDL_Window *window)
	{
		this->targetWindow = window;

		direct3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (direct3d == NULL)
		{
			LOG_ERROR("Failed to create Direct3D 9 object");
			return false;
		}

		D3DADAPTER_IDENTIFIER9 adapterIdentifier = {};
		direct3d->GetAdapterIdentifier(0, 0, &adapterIdentifier);

		LOG_INFO_ARGS("Adapter: %s", adapterIdentifier.Description);

		presentParameters.Windowed = TRUE;
		presentParameters.BackBufferWidth = 1920;
		presentParameters.BackBufferHeight = 1080;
		presentParameters.SwapEffect = D3DSWAPEFFECT::D3DSWAPEFFECT_COPY;
		presentParameters.BackBufferFormat = D3DFORMAT::D3DFMT_A8R8G8B8;
		presentParameters.PresentationInterval = 0;
		presentParameters.EnableAutoDepthStencil = FALSE;

		SDL_SysWMinfo wmInfo = {};
		SDL_GetWindowWMInfo(targetWindow, &wmInfo);

		presentParameters.hDeviceWindow = wmInfo.info.win.window;

		HRESULT result = S_OK;
		if ((result = direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL,
											 D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParameters, &device)) != S_OK)
		{
			LOG_ERROR_ARGS("Failed to create Direct3D 9 device. Error: 0x%X", result);
			return false;
		}

		if ((result = device->GetSwapChain(0, &swapChain)) != S_OK)
		{
			LOG_ERROR_ARGS("Failed to get Direct3D 9 device swap chain. Error: 0x%X", result);
			return false;
		}

		i32 width = 0;
		i32 height = 0;
		SDL_GetWindowSize(targetWindow, &width, &height);

		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = width;
		windowRect.bottom = height;

		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinZ = 0.0f;
		viewport.MaxZ = 1.0f;
		device->SetViewport(&viewport);

		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL_NONE);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ZENABLE, D3DZB_FALSE);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ZWRITEENABLE, D3DZB_FALSE);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ZFUNC, D3DCMP_ALWAYS);
		
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_SRCBLEND, D3DBLEND::D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLEND, D3DBLEND::D3DBLEND_INVSRCALPHA);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_SRCBLENDALPHA, D3DBLEND::D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLENDALPHA, D3DBLEND::D3DBLEND_INVSRCALPHA);

		return true;
	}

	void Backend_D3D9::Destroy()
	{
		device->Release();
		direct3d->Release();
	}

	void Backend_D3D9::Clear(u32 flags, Color color, float depth, u8 stencil)
	{
		DWORD clearFlags = 0;

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

		device->Clear(0, NULL, clearFlags, clearColor, depth, static_cast<DWORD>(stencil));
		device->BeginScene();
	}

	void Backend_D3D9::SwapBuffers()
	{
		device->EndScene();
		device->Present(&windowRect, NULL, presentParameters.hDeviceWindow, NULL);
		ValidateRect(presentParameters.hDeviceWindow, &windowRect);
	}
	
	void Backend_D3D9::Begin()
	{
	}
	
	void Backend_D3D9::End()
	{
	}
	
	void Backend_D3D9::GetViewportSize(float* x, float* y, float* width, float* height)
	{
		*x = static_cast<float>(viewport.X);
		*y = static_cast<float>(viewport.Y);
		*width = static_cast<float>(viewport.Width);
		*height = static_cast<float>(viewport.Height);
	}
	
	void Backend_D3D9::ResizeMainRenderTarget(u32 newWidth, u32 newHeight)
	{
		vertexBufferSet = false;
		indexBufferSet = false;
		currentVertexDescSize = 0;
		shaderSet = false;

		viewport.Width = static_cast<DWORD>(newWidth);
		viewport.Height = static_cast<DWORD>(newHeight);

		device->SetViewport(&viewport);

		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = static_cast<LONG>(newWidth);
		windowRect.bottom = static_cast<LONG>(newHeight);
	}
	
	void Backend_D3D9::SetBlendState(const BlendState* state)
	{
	}
	
	Buffer* Backend_D3D9::CreateVertexBuffer(BufferUsage usage, const void* initialData, u32 size)
	{
		Buffer_D3D9* buffer = new Buffer_D3D9(device);

		if (buffer->InitializeVertex(usage, size, initialData))
		{
			return buffer;
		}

		delete[] buffer;
		return nullptr;
	}
	
	Buffer* Backend_D3D9::CreateIndexBuffer(BufferUsage usage, IndexFormat format, const void* initialData, u32 size)
	{
		Buffer_D3D9* buffer = new Buffer_D3D9(device);
		
		if (buffer->InitializeIndex(usage, format, size, initialData))
		{
			return buffer;
		}

		delete[] buffer;
		return nullptr;
	}
	
	void Backend_D3D9::DestroyBuffer(Buffer* buffer)
	{
		if (buffer != nullptr)
		{
			Buffer_D3D9* d3dBuffer = static_cast<Buffer_D3D9*>(buffer);
			d3dBuffer->Destroy();
			delete[] buffer;
		}
	}
	
	void Backend_D3D9::BindVertexBuffer(Buffer* buffer)
	{
		if (buffer != nullptr && currentVertexDescSize != 0)
		{
			Buffer_D3D9* d3dBuffer = static_cast<Buffer_D3D9*>(buffer);

			if (d3dBuffer->GetType() != BufferType::BUFFER_VERTEX)
			{
				return;
			}

			d3dBuffer->Bind(currentVertexDescSize);
			vertexBufferSet = true;
		}
		else
		{
			vertexBufferSet = false;
		}

	}
	
	void Backend_D3D9::BindIndexBuffer(Buffer* buffer)
	{
		if (buffer != nullptr)
		{
			Buffer_D3D9* d3dBuffer = static_cast<Buffer_D3D9*>(buffer);

			if (d3dBuffer->GetType() != BufferType::BUFFER_INDEX)
			{
				return;
			}

			d3dBuffer->Bind(0);
			indexBufferSet = true;
		}
		else
		{
			indexBufferSet = false;
		}
	}
	
	void Backend_D3D9::SetBufferData(Buffer* buffer, const void* src, u32 offset, u32 size)
	{
		if (buffer != nullptr)
		{
			Buffer_D3D9* d3dBuffer = static_cast<Buffer_D3D9*>(buffer);
			d3dBuffer->SetData(src, offset, size);
		}
	}
	
	void* Backend_D3D9::MapBuffer(Buffer* buffer, BufferMapping mapMode)
	{
		return nullptr;
	}
	
	void Backend_D3D9::UnmapBuffer(Buffer* buffer)
	{
	}
	
	Shader* Backend_D3D9::CreateShader(const u8* vsSource, u32 vsSourceLength, const u8* fsSource, u32 fsSourceLength)
	{
		Shader_D3D9* shader = new Shader_D3D9(device);

		if (shader->Initialize(vsSource, vsSourceLength, fsSource, fsSourceLength))
		{
			return shader;
		}

		delete[] shader;
		return nullptr;
	}
	
	void Backend_D3D9::DestroyShader(Shader* shader)
	{
		if (shader != nullptr)
		{
			Shader_D3D9* d3dShader = static_cast<Shader_D3D9*>(shader);
			d3dShader->Destroy();
			delete[] shader;
		}
	}
	
	void Backend_D3D9::BindShader(Shader* shader)
	{
		if (shader != nullptr)
		{
			Shader_D3D9* d3dShader = static_cast<Shader_D3D9*>(shader);
			d3dShader->Bind();
			shaderSet = true;
		}
		else
		{
			shaderSet = false;
		}
	}
	
	void Backend_D3D9::SetShaderMatrix(u32 index, const float* matrix)
	{
		if (shaderSet)
		{
			device->SetVertexShaderConstantF(index, matrix, 4);
		}	
	}
	
	void Backend_D3D9::SetShaderMatrix(u32 index, const mat4* matrix)
	{
		if (shaderSet)
		{
			mat4 transposedMatrix = glm::transpose(*matrix);
			device->SetVertexShaderConstantF(index, glm::value_ptr(transposedMatrix), 4);
		}
	}
	
	VertexDescription* Backend_D3D9::CreateVertexDescription(const VertexAttribute* attribs, u32 attribCount, u32 stride, const Shader* shader)
	{
		VertexDescription_D3D9* vertexDesc = new VertexDescription_D3D9(device);

		if (vertexDesc->Initialize(attribs, attribCount, stride))
		{
			return vertexDesc;
		}

		delete[] vertexDesc;
		return nullptr;
	}
	
	void Backend_D3D9::DestroyVertexDescription(VertexDescription* desc)
	{
		if (desc != nullptr)
		{
			VertexDescription_D3D9* vertexDesc = static_cast<VertexDescription_D3D9*>(desc);
			vertexDesc->Destroy();
			delete[] desc;
		}
	}
	
	void Backend_D3D9::SetVertexDescription(VertexDescription* desc)
	{
		if (desc != nullptr)
		{
			VertexDescription_D3D9* vertexDesc = static_cast<VertexDescription_D3D9*>(desc);
			vertexDesc->Set();

			currentVertexDescSize = static_cast<UINT>(vertexDesc->GetVertexStride());
		}
		else
		{
			currentVertexDescSize = 0;
		}
	}
	
	Texture* Backend_D3D9::CreateTexture(u32 width, u32 height, TextureFormat format, u32 flags)
	{
		Texture_D3D9* texture = new Texture_D3D9(device);

		if (texture->Initialize(width, height, format, flags))
		{
			return texture;
		}

		delete[] texture;
		return nullptr;
	}
	
	void Backend_D3D9::DestroyTexture(Texture* texture)
	{
		if (texture != nullptr)
		{
			Texture_D3D9* d3dTexture = static_cast<Texture_D3D9*>(texture);
			d3dTexture->Destroy();
			delete[] d3dTexture;
		}
	}
	
	void Backend_D3D9::BindTexture(Texture* texture, u32 unit)
	{
		if (texture != nullptr)
		{
			Texture_D3D9* d3dTexture = static_cast<Texture_D3D9*>(texture);
			d3dTexture->Bind(unit);
		}
	}
	
	void Backend_D3D9::SetTextureData(Texture* texture, const void* data)
	{
		if (texture != nullptr)
		{
			Texture_D3D9* d3dTexture = static_cast<Texture_D3D9*>(texture);
			d3dTexture->SetData(data);
		}
	}
	
	void Backend_D3D9::SetTextureData(Texture* texture, const void* data, u32 x, u32 y, u32 width, u32 height)
	{
		if (texture != nullptr)
		{
			Texture_D3D9* d3dTexture = static_cast<Texture_D3D9*>(texture);
			d3dTexture->SetData(data, x, y, width, height);
		}
	}
	
	void Backend_D3D9::DrawArrays(PrimitiveType type, i32 firstVertex, i32 vertexCount)
	{
		if (vertexBufferSet && shaderSet && currentVertexDescSize != 0)
		{
			D3DPRIMITIVETYPE primType = GFXtoD3D9::PrimitiveTypes[static_cast<i32>(type)];
			UINT primCount = vertexCount / GFXtoD3D9::VerticesPerPrimitive[static_cast<i32>(type)];
			device->DrawPrimitive(primType, firstVertex, primCount);
		}
	}
	
	void Backend_D3D9::DrawIndexed(PrimitiveType type, i32 vertexCount, i32 firstIndex, i32 indexCount)
	{
		if (vertexBufferSet && indexBufferSet && shaderSet && currentVertexDescSize != 0)
		{
			D3DPRIMITIVETYPE primType = GFXtoD3D9::PrimitiveTypes[static_cast<i32>(type)];
			UINT primCount = indexCount / GFXtoD3D9::VerticesPerPrimitive[static_cast<i32>(type)];
			device->DrawIndexedPrimitive(primType, 0, 0, vertexCount, firstIndex, primCount);
		}
	}
}
