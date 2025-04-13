#include "Light.h"

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
    float4 size;
    float4 color;
};

struct VSInput
{
    float3 pos : POSITION;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
};

VSOutput VS(VSInput vertex)
{
    VSOutput result;
    
    result.pos = mul(vp, mul(model, float4(vertex.pos, 1.0)));
    result.worldPos = mul(model, float4(vertex.pos, 1.0)).xyz;
    
    return result;
}
