#pragma once
#include <d3d9.h>
#include "../shader.h"

namespace GFX::LowLevel::D3D9
{
	class Shader_D3D9 : public Shader
	{
	public:
		Shader_D3D9() = delete;
		Shader_D3D9(LPDIRECT3DDEVICE9 d3dDevice) : device(d3dDevice) {}

		//GLuint GetVertexHandle() const;
		//GLuint GetFragmentHandle() const;

		bool Initialize(const u8 *vsSource, u32 vsSourceSize, const u8 *fsSource, u32 fsSourceSize);
		void Destroy();

		void Bind();
	private:
		LPDIRECT3DDEVICE9 device = NULL;

		LPDIRECT3DVERTEXSHADER9 vertShader = NULL;
		LPDIRECT3DPIXELSHADER9 fragShader = NULL; // I AIN'T USING DIRECT3D TERMINOLOGY INSTEAD OF OPENGL ONE
	};
}
