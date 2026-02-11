struct FSInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Color : COLOR0;
};

Texture2D Texture : register(t0);
sampler TextureSampler : register(s0);

float4 main(FSInput input) : SV_Target0
{
	return Texture.Sample(TextureSampler, input.TexCoord) * input.Color;
}