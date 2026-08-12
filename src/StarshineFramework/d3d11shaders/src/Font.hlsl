#ifndef FONT_HLSL
#define FONT_HLSL

float4 GetSolidColor(float alpha, float4 fragColor)
{
	return float4(fragColor.rgb, fragColor.a * alpha);
}

float4 GetFillColor_OutlineRG(float4 texelColor, float4 fillColor)
{
	float fillMask = texelColor.r;
	return GetSolidColor(fillMask, fillColor);
}

float4 GetOutlineColor_OutlineRG(float4 texelColor, float4 outlineColor)
{
	float fillMask = texelColor.g;
	return GetSolidColor(fillMask, outlineColor);
}

#endif
