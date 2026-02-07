#pragma once
#include "D3D9Device.h"
#include <d3d9.h>

namespace Starshine::Rendering::D3D9
{
	struct Shader_D3D9 : public Shader
	{
		Shader_D3D9(IDirect3DDevice9* device, const DWORD* vsBytecode, const DWORD* fsBytecode);
		~Shader_D3D9();

		void SetVertexShaderVariables(u32 index, size_t count, const f32* values);
		void SetFragmentShaderVariables(u32 index, size_t count, const f32* values);

		void SetVertexShaderMatrix(u32 index, const mat4& matrix);
		void SetFragmentShaderMatrix(u32 index, const mat4& matrix);

		IDirect3DDevice9* Device{};

		IDirect3DVertexShader9* VertexShader{};
		IDirect3DPixelShader9* FragmentShader{};
	};
}
