#include <SDL2/SDL_syswm.h>
#include "d3d9_backend.h"
#include "d3d9_buffers.h"
#include "d3d9_shader.h"
#include "d3d9_vertex_desc.h"
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
		presentParameters.SwapEffect = D3DSWAPEFFECT::D3DSWAPEFFECT_DISCARD;
		//presentParameters.BackBufferCount = 2;
		presentParameters.BackBufferFormat = D3DFORMAT::D3DFMT_A8R8G8B8;
		presentParameters.PresentationInterval = 0;
		presentParameters.EnableAutoDepthStencil = FALSE;

		SDL_SysWMinfo wmInfo = {};
		SDL_GetWindowWMInfo(targetWindow, &wmInfo);

		HRESULT result = S_OK;
		if ((result = direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wmInfo.info.win.window,
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

		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_CULLMODE, D3DCULL_NONE);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ZENABLE, D3DZB_FALSE);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ZWRITEENABLE, D3DZB_FALSE);
		device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_ZFUNC, D3DCMP_ALWAYS);
		//device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_BLEND, D3DCMP_ALWAYS);

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
		swapChain->Present(NULL, NULL, NULL, NULL, D3DPRESENT_INTERVAL_ONE);
	}
	
	void Backend_D3D9::Begin()
	{
	}
	
	void Backend_D3D9::End()
	{
	}
	
	void Backend_D3D9::GetViewportSize(float* x, float* y, float* width, float* height)
	{
	}
	
	void Backend_D3D9::ResizeMainRenderTarget(u32 newWidth, u32 newHeight)
	{
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
		return nullptr;
	}
	
	void Backend_D3D9::DestroyTexture(Texture* texture)
	{
		
	}
	
	void Backend_D3D9::BindTexture(const Texture* texture, u32 unit)
	{
		
	}
	
	void Backend_D3D9::SetTextureData(Texture* texture, const void* data)
	{
		
	}
	
	void Backend_D3D9::SetTextureData(Texture* texture, const void* data, u32 x, u32 y, u32 width, u32 height)
	{
		
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
