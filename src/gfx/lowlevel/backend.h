#pragma once
#include <SDL2/SDL.h>
#include "../../common/int_types.h"
#include "../../common/color.h"
#include "buffers.h"
#include "shader.h"
#include "vertex_desc.h"
#include "texture.h"
#include "blend_state.h"
#include <string>

namespace GFX
{
	namespace LowLevel
	{
		using Common::Color;

		enum class BackendType
		{
			BACKEND_NONE = -1,
			BACKEND_OPENGL_ARB,
			BACKEND_D3D9,
			BACKEND_COUNT
		};

		enum ClearFlags : u32
		{
			GFX_CLEAR_COLOR = 1,
			GFX_CLEAR_DEPTH = 2,
			GFX_CLEAR_STENCIL = 4
		};

		enum class PrimitiveType
		{
			PRIMITIVE_POINTS,

			PRIMITIVE_LINES,
			PRIMITIVE_LINE_STRIP,

			PRIMITIVE_TRIANGLES,
			PRIMITIVE_TRIANGLE_STRIP,
			PRIMITIVE_TRIANGLE_FAN
		};

		constexpr const char* BackendNames[static_cast<int>(BackendType::BACKEND_COUNT)] = 
		{
			"OpenGL_ARB",
			"D3D9"
		};

		class Backend
		{
		public:
			Backend() {}

			virtual bool Initialize(SDL_Window* window) = 0;
			virtual void Destroy() = 0;

			virtual void Clear(u32 flags, Color color, float depth, u8 stencil) = 0;
			virtual void SwapBuffers() = 0;

			virtual void Begin() = 0;
			virtual void End() = 0;

			virtual void GetViewportSize(float* x, float* y, float* width, float* height) = 0;
			virtual void ResizeMainRenderTarget(u32 newWidth, u32 newHeight) = 0;

			virtual void SetBlendState(const BlendState* state) = 0;

			virtual Buffer* CreateVertexBuffer(BufferUsage usage, const void* initialData, u32 size) = 0;
			virtual Buffer* CreateIndexBuffer(BufferUsage usage, IndexFormat format, const void* initialData, u32 size) = 0;
			virtual void DestroyBuffer(Buffer* buffer) = 0;

			virtual void BindVertexBuffer(Buffer* buffer) = 0;
			virtual void BindIndexBuffer(Buffer* buffer) = 0;

			virtual void SetBufferData(Buffer* buffer, const void* src, u32 offset, u32 size) = 0;
			virtual void* MapBuffer(Buffer* buffer, BufferMapping mapMode) = 0;
			virtual void UnmapBuffer(Buffer* buffer) = 0;

			virtual Shader* CreateShader(const u8* vsSource, u32 vsSourceLength, const u8* fsSource, u32 fsSourceLength) = 0;
			virtual void DestroyShader(Shader* shader) = 0;

			virtual void BindShader(Shader* shader) = 0;

			virtual void SetShaderMatrix(u32 index, const float* matrix) = 0;
			virtual void SetShaderMatrix(u32 index, const mat4* matrix) = 0;

			virtual VertexDescription* CreateVertexDescription(const VertexAttribute* attribs, u32 attribCount, u32 stride, const Shader* shader) = 0;
			virtual void DestroyVertexDescription(VertexDescription* desc) = 0;

			virtual void SetVertexDescription(VertexDescription* desc) = 0;

			virtual Texture* CreateTexture(u32 width, u32 height, TextureFormat format, u32 flags) = 0;
			virtual void DestroyTexture(Texture* texture) = 0;

			virtual void BindTexture(Texture* texture, u32 unit) = 0;

			virtual void SetTextureData(Texture* texture, const void* data) = 0;
			virtual void SetTextureData(Texture* texture, const void* data, u32 x, u32 y, u32 width, u32 height) = 0;

			virtual void DrawArrays(PrimitiveType type, i32 firstVertex, i32 vertexCount) = 0;
			virtual void DrawIndexed(PrimitiveType type, i32 vertexCount, i32 firstIndex, i32 indexCount) = 0;

			BackendType GetType() { return type; }
		protected:
			BackendType type = BackendType::BACKEND_NONE;
			bool initialized = false;

			SDL_Window* targetWindow = nullptr;
		};
	}
}
