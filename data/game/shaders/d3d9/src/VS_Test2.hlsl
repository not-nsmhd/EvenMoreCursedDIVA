struct VSInput
{
	float2 Position : POSITION;
	float4 Color : COLOR0;
};

struct VSOutput
{
	float4 Color : COLOR0;
	float4 Position : POSITION;
};

float4x4 vs_ProjMatrix = 
{
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

VSOutput main(VSInput input)
{
	VSOutput output;
	
	output.Position = mul(float4(input.Position.xy, 0.0, 1.0), vs_ProjMatrix);
	output.Color = input.Color;
	
	return output;
}
