#include "Light.h"

struct VSOutput
{
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
    float4 color : COLOR;
};

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
    float4 color;
};

float4 PS(VSOutput pixel) : SV_Target0
{
    return float4(CalculateColor(color.xyz, float3(1, 0, 0), pixel.worldPos.xyz, 0.0, true), color.w);
}
