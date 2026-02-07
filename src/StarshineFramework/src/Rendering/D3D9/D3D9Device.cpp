#include <d3d9.h>
#include <SDL2/SDL_syswm.h>
#include "Common/MathExt.h"
#include "Common/Logging/Logging.h"
#include "Rendering/Types.h"
#include "D3D9Device.h"
#include "D3D9Common.h"
#include "D3D9Buffers.h"
#include "D3D9VertexDesc.h"
#include "D3D9Shader.h"
//#include "OpenGLTexture.h"
#include <array>
#include "IO/Xml.h"
#include "IO/Path/File.h"

namespace Starshine::Rendering::D3D9
{
	using std::array;
	using std::string_view;

	constexpr const char* LogName = "Starshine::Rendering::D3D9";

	struct D3D9Device::Impl
	{
		SDL_Window* GameWindow{ nullptr };

		IDirect3D9* Direct3D9{ nullptr };
		IDirect3DDevice9* BaseDevice{ nullptr };
		D3DPRESENT_PARAMETERS PresentParams{};

		bool VertexBufferSet{ false };
		bool VertexDescSet{ false };
		UINT CurrentVertexStride{};

		bool IndexBufferSet{ false };
		//GLenum CurrentIndexFormat = 0;

		bool ShaderSet = false;
		bool BeginCalled = false;

		bool Initialize(SDL_Window* gameWindow)
		{
			u32 windowFlags = SDL_GetWindowFlags(gameWindow);
			if ((windowFlags & SDL_WINDOW_OPENGL) != 0)
			{
				LogError(LogName, "D3D9 device cannot be created for an OpenGL window");
				return false;
			}

			GameWindow = gameWindow;
			HRESULT result = D3D_OK;

			Direct3D9 = Direct3DCreate9(D3D_SDK_VERSION);
			if (Direct3D9 == NULL) { return false; }

			SDL_SysWMinfo wmInfo{};
			SDL_GetWindowWMInfo(GameWindow, &wmInfo);

			PresentParams.Windowed = TRUE;
			PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;

			if ((result = Direct3D9->CreateDevice(0, D3DDEVTYPE_HAL, wmInfo.info.win.window,
				D3DCREATE_HARDWARE_VERTEXPROCESSING, &PresentParams, &BaseDevice)) != D3D_OK)
			{
				LogError(LogName, "Failed to create a Direct3D 9 device. Error: 0x%08X", result);
				return false;
			}

			return true;
		}

		void Destroy()
		{
			BaseDevice->Release();
			Direct3D9->Release();
		}

		void SwapBuffers()
		{
			if (BeginCalled)
			{
				BaseDevice->EndScene();
				BeginCalled = false;
			}
			BaseDevice->Present(NULL, NULL, NULL, NULL);
		}

		void DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
		{
			if (VertexBufferSet && VertexDescSet && ShaderSet)
			{
				if (!BeginCalled)
				{
					BaseDevice->BeginScene();
					BeginCalled = true;
				}

				D3DPRIMITIVETYPE primType = ConversionTables::D3DPrimitiveTypes[static_cast<size_t>(type)];
				u32 vtxPerPrim = ConversionTables::D3DVerticesPerPrimitive[static_cast<size_t>(type)];
				BaseDevice->DrawPrimitive(primType, firstVertex, vertexCount / vtxPerPrim);
			}
		}

		void DrawIndexed(PrimitiveType type, u32 firstIndex, u32 vertexCount, u32 indexCount)
		{
			if (VertexBufferSet && IndexBufferSet && VertexDescSet && ShaderSet)
			{
				if (!BeginCalled)
				{
					BaseDevice->BeginScene();
					BeginCalled = true;
				}

				D3DPRIMITIVETYPE primType = ConversionTables::D3DPrimitiveTypes[static_cast<size_t>(type)];
				u32 vtxPerPrim = ConversionTables::D3DVerticesPerPrimitive[static_cast<size_t>(type)];
				BaseDevice->DrawIndexedPrimitive(primType, 0, 0, vertexCount, firstIndex, indexCount / vtxPerPrim);
			}
		}

		void SetVertexBuffer(const VertexBuffer_D3D9* buffer)
		{
			if (buffer == nullptr)
			{
				BaseDevice->SetStreamSource(0, NULL, 0, 0);
				VertexBufferSet = false;
			}
			else if (VertexDescSet)
			{
				BaseDevice->SetStreamSource(0, buffer->BaseBuffer, 0, CurrentVertexStride);
				VertexBufferSet = true;
			}
		}

		void SetVertexDesc(const VertexDesc_D3D9* desc)
		{
			if (desc == nullptr)
			{
				BaseDevice->SetVertexDeclaration(NULL);
				VertexDescSet = false;
			}
			else
			{
				BaseDevice->SetVertexDeclaration(desc->BaseDeclaration);
				CurrentVertexStride = desc->VertexStride;
				VertexDescSet = true;
			}
		}

		void SetIndexBuffer(const IndexBuffer_D3D9* buffer)
		{
			if (buffer == nullptr)
			{
				BaseDevice->SetIndices(NULL);
				IndexBufferSet = false;
			}
			else
			{
				BaseDevice->SetIndices(buffer->BaseBuffer);
				IndexBufferSet = true;
			}
		}
	};

	D3D9Device::D3D9Device() : impl(std::make_unique<Impl>())
	{
	}

	D3D9Device::~D3D9Device()
	{
	}

	bool D3D9Device::Initialize(SDL_Window* gameWindow)
	{
		return impl->Initialize(gameWindow);
	}

	void D3D9Device::Destroy()
	{
		impl->Destroy();
	}

	RectangleF D3D9Device::GetViewportSize() const
	{
		RectangleF viewport = {};
		//glGetFloatv(GL_VIEWPORT, &viewport.X);
		return viewport;
	}

	void D3D9Device::Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil)
	{
		DWORD clearFlags{};
		D3DCOLOR clearColor{};

		if ((flags & ClearFlags::ClearFlags_Color) != 0)
		{
			clearColor = D3DCOLOR_RGBA(color.R, color.G, color.B, color.A);
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

		impl->BaseDevice->Clear(0, NULL, clearFlags, clearColor, depth, stencil);
	}

	void D3D9Device::SwapBuffers()
	{
		impl->SwapBuffers();
	}

	void D3D9Device::SetBlendState(bool enable, BlendFactor srcColor, BlendFactor destColor, BlendFactor srcAlpha, BlendFactor destAlpha)
	{
		/*GLboolean blendEnabled = glIsEnabled(GL_BLEND);
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

		glBlendFuncSeparate(glSrcColor, glDstColor, glSrcAlpha, glDstAlpha);*/
	}

	void D3D9Device::SetBlendOperation(BlendOperation op)
	{
		//GLenum glOperation = ConversionTables::GLBlendOperations[static_cast<size_t>(op)];
		//glBlendEquation(glOperation);
	}

	void D3D9Device::SetFaceCullingState(bool enable, PolygonOrientation frontFaceOrientation, Face facesToCull)
	{
		/*GLboolean cullingEnabled = glIsEnabled(GL_CULL_FACE);
		if (enable)
		{
			if (!cullingEnabled) { glEnable(GL_CULL_FACE); }
		}
		else
		{
			if (cullingEnabled) { glDisable(GL_CULL_FACE); }
			return;
		}

		GLenum glOrientation = ConversionTables::GLPolygonOrientation[static_cast<size_t>(frontFaceOrientation)];
		GLenum glFacesToCull = ConversionTables::GLFace[static_cast<size_t>(facesToCull)];

		glFrontFace(glOrientation);
		glCullFace(glFacesToCull);*/
	}

	void D3D9Device::DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
	{
		impl->DrawArrays(type, firstVertex, vertexCount);
	}

	void D3D9Device::DrawIndexed(PrimitiveType type, u32 firstIndex, u32 vertexCount, u32 indexCount)
	{
		impl->DrawIndexed(type, firstIndex, vertexCount, indexCount);
	}

	std::unique_ptr<VertexBuffer> D3D9Device::CreateVertexBuffer(size_t size, const void* initialData, bool dynamic)
	{
		if (!dynamic && initialData == nullptr || size == 0)
		{
			return nullptr;
		}

		std::unique_ptr<VertexBuffer_D3D9> buffer = std::make_unique<VertexBuffer_D3D9>(impl->BaseDevice, initialData, size, dynamic);
		return buffer;
	}

	std::unique_ptr<IndexBuffer> D3D9Device::CreateIndexBuffer(size_t size, IndexFormat format, const void* initialData, bool dynamic)
	{
		if (!dynamic && initialData == nullptr || size == 0)
		{
			return nullptr;
		}

		std::unique_ptr<IndexBuffer_D3D9> buffer = std::make_unique<IndexBuffer_D3D9>(impl->BaseDevice, initialData, size, format, dynamic);
		return buffer;
	}

	std::unique_ptr<VertexDesc> D3D9Device::CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount)
	{
		if (attribs == nullptr || attribCount == 0)
		{
			return nullptr;
		}

		std::unique_ptr<VertexDesc_D3D9> desc = std::make_unique<VertexDesc_D3D9>(impl->BaseDevice, attribs, attribCount);
		return desc;
	}

	std::unique_ptr<Shader> D3D9Device::LoadShader(const void* vsData, size_t vsSize, const void* fsData, size_t fsSize)
	{
		if (vsData == nullptr || vsSize == 0 || fsData == nullptr || fsSize == 0)
		{
			return nullptr;
		}

		std::unique_ptr<Shader_D3D9> shader = std::make_unique<Shader_D3D9>(impl->BaseDevice,
			reinterpret_cast<const DWORD*>(vsData), reinterpret_cast<const DWORD*>(fsData));

		return shader;
	}

	std::unique_ptr<Texture> D3D9Device::CreateTexture(u32 width, u32 height, GFX::TextureFormat format, bool nearestFilter, bool repeat)
	{
		/*if (width == 0 || height == 0) { return nullptr; }
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

		std::unique_ptr<Texture_OpenGL> texture = std::make_unique<Texture_OpenGL>(*this, width, height, format, false);
		texture->Handle = texHandle;
		texture->Filter = filter;
		texture->WrapMode = wrapMode;

		return texture;*/
		return nullptr;
	}

	void D3D9Device::SetVertexBuffer(const VertexBuffer* buffer, const VertexDesc* desc)
	{
		if (buffer == nullptr || desc == nullptr)
		{
			impl->SetVertexBuffer(nullptr);
			impl->SetVertexDesc(nullptr);
		}
		else
		{
			const VertexDesc_D3D9* d3dDesc = static_cast<const VertexDesc_D3D9*>(desc);
			const VertexBuffer_D3D9* d3dBuffer = static_cast<const VertexBuffer_D3D9*>(buffer);

			impl->SetVertexBuffer(d3dBuffer);
			impl->SetVertexDesc(d3dDesc);
		}
	}

	void D3D9Device::SetIndexBuffer(const IndexBuffer* buffer)
	{
		if (buffer == nullptr)
		{
			impl->SetIndexBuffer(nullptr);
		}
		else
		{
			const IndexBuffer_D3D9* d3dBuffer = static_cast<const IndexBuffer_D3D9*>(buffer);
			impl->SetIndexBuffer(d3dBuffer);
		}
	}

	void D3D9Device::SetShader(const Shader* shader)
	{
		if (shader == nullptr)
		{
			impl->BaseDevice->SetVertexShader(NULL);
			impl->BaseDevice->SetPixelShader(NULL);
			impl->ShaderSet = false;
		}
		else
		{
			const Shader_D3D9* d3dShader = static_cast<const Shader_D3D9*>(shader);
			impl->BaseDevice->SetVertexShader(d3dShader->VertexShader);
			impl->BaseDevice->SetPixelShader(d3dShader->FragmentShader);
			impl->ShaderSet = true;
		}
	}

	void D3D9Device::SetTexture(const Texture* texture, u32 slot)
	{
		/*glActiveTextureARB(GL_TEXTURE0_ARB + slot);
		if (texture == nullptr)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			const Texture_OpenGL* glTexture = static_cast<const Texture_OpenGL*>(texture);
			glBindTexture(GL_TEXTURE_2D, glTexture->Handle);
		}*/
	}
}
