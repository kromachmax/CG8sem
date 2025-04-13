#include "Light.h"

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
    float4x4 norm;
    float4 shine; // x - shininess
};

Texture2D colorTexture : register(t0);

SamplerState colorSampler : register(s0);

struct VSOutput
{
    float4 pos : SV_Position;
    float4 worldPos : POSITION;
    float3 tang : TANGENT;
    float3 norm : NORMAL;
    float2 uv : TEXCOORD;
};

float4 PS(VSOutput pixel) : SV_Target0
{
    float3 color = colorTexture.Sample(colorSampler, pixel.uv).xyz;

    float3 normal = float3(0, 0, 0);
    normal = pixel.norm;
    

    return float4(CalculateColor(color, normal, pixel.worldPos.xyz, shine.x, false), 1.0);
    
    //return float4(lights[0].color.xyz, 1.0);
}
