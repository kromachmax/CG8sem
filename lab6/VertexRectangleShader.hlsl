#include "Light.h"

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
};

struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
    float4 color : COLOR;
};

VSOutput VS(VSInput vertex)
{
    VSOutput result;

    result.pos = mul(vp, mul(model, float4(vertex.pos, 1.0)));
    result.worldPos = mul(model, float4(vertex.pos, 1.0)).xyz;
    result.color = vertex.color;

    return result;
}
