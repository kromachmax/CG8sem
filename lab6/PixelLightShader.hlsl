#include "Light.h"

struct VSOutput
{
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
};

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
    float4 size;
    float4 color;
};

float4 PS(VSOutput pixel) : SV_Target0
{
    return color;
}
