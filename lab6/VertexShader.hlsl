#include "Light.h"

cbuffer GeomBuffer : register(b1)
{
    float4x4 model; 
    float4x4 norm; 
    float4 shine;
}

struct VSInput
{
    float3 pos : POSITION;
    float3 tang : TANGENT;
    float3 norm : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_Position; 
    float4 worldPos : POSITION; 
    float3 tang : TANGENT; 
    float3 norm : NORMAL; 
    float2 uv : TEXCOORD; 
};

VSOutput VS(VSInput vertex)
{
    VSOutput result = (VSOutput) 0;
    
    float4 worldPos = mul(model, float4(vertex.pos, 1.0));
    
    result.pos = mul(vp, worldPos);
    
    result.worldPos = worldPos;
    
    result.uv = vertex.uv;
    
    result.tang = mul(norm, float4(vertex.tang, 0.0)).xyz;
    result.norm = mul(norm, float4(vertex.norm, 0.0)).xyz;

    return result;
}