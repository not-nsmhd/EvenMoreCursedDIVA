struct VSInput
{
	float2 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
	float2 TexCoord : TEXCOORD0;
	float4 Color : COLOR0;
	float4 Position : POSITION;
};

float4x4 g_TransformMatrix;

VSOutput main(VSInput input)
{
	VSOutput output;
	
	output.Position = mul(g_TransformMatrix, float4(input.Position.xy, 0.0, 1.0));
	output.Color = input.Color;
	output.TexCoord = input.TexCoord;
	
	return output;
}
