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
    float3 localPos : POSITION1;
};

VSOutput VS(VSInput vertex)
{
    VSOutput result;

    float3 pos = cameraPos.xyz + vertex.pos * size.x;

    result.pos = mul(vp, mul(model, float4(pos, 1.0)));
    result.pos.z = result.pos.w;
    result.localPos = vertex.pos;

    return result;
}
