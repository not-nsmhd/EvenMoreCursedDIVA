struct PSInput
{
	float2 TexCoord : TEXCOORD0;
	float4 Color : COLOR0;
	float2 Position : POSITION;
};

texture g_SpriteTexture;
sampler SpriteTextureSampler = sampler_state { Texture = <g_SpriteTexture>; };

float4 main(PSInput input) : COLOR0
{
	float4 texColor = tex2D(SpriteTextureSampler, input.TexCoord);
	return float4(texColor.bgra) * input.Color;
}
