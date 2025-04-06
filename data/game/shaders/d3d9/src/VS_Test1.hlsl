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

VSOutput main(VSInput input)
{
	VSOutput output;
	
	output.Position = float4(input.Position.xy, 0.0, 1.0);
	output.Color = input.Color;
	
	return output;
}
