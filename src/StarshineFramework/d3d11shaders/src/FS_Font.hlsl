#include "Font.hlsl"

#define FONTTYPE_PLAINRGBA 0
#define FONTTYPE_SINGLECHANNEL 1
#define FONTTYPE_OUTLINE_RG 2

struct FSInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Color : COLOR0;
};

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

cbuffer FontUniforms : register(b0)
{
	int U_FontType = FONTTYPE_PLAINRGBA;
	float4 U_OutlineColor;
};

float4 main(FSInput input) : SV_Target0
{
	float4 texel = Texture.Sample(TextureSampler, input.TexCoord);

	switch (U_FontType)
	{
	case FONTTYPE_PLAINRGBA:
		return texel * input.Color;
	case FONTTYPE_SINGLECHANNEL:
		return GetSolidColor(texel.r, input.Color);
	case FONTTYPE_OUTLINE_RG:
		float4 fill = GetSolidColor(texel.r, input.Color) * texel.r;
		float4 outline = float4(U_OutlineColor.rgb, texel.g);
		return outline + fill;
	}
	return float4(0.0, 0.0, 0.0, 0.0);
}