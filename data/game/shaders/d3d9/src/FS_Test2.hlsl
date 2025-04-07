struct PSInput
{
	float4 Color : COLOR0;
	float2 Position : POSITION;
};

float4 main(PSInput input) : COLOR0
{
	return input.Color;
}
