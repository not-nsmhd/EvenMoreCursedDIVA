struct VSInput
{
	float2 Position : POSITION;
	float4 Color : COLOR0;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

float4x4 vs_TransformMatrix : register(vs, c[0]);

VSOutput main(VSInput input)
{
	VSOutput output;

	float4 pos = float4(input.Position.xy, 0.0, 1.0);
	output.Position = mul(pos, vs_TransformMatrix);
	output.Color = input.Color;

	return output;
}