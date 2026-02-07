struct FSInput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

float4 main(FSInput input) : COLOR0
{
	return input.Color;
}