struct FSInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Color : COLOR0;
};

texture fs_Texture;
sampler fs_TextureSampler = sampler_state
{
	Texture = fs_Texture;
};

float4 main(FSInput input) : SV_Target0
{
	return input.Color;
}