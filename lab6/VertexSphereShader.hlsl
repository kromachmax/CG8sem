#include "Light.h"

cbuffer ObjectData : register(b1)
{
    float4x4 worldMatrix;
    float4 scaleFactor;
    float4 tint;
};

struct VertexInput
{
    float3 position : POSITION;
};

struct PixelInput
{
    float4 screenPos : SV_Position;
    float3 objectSpacePos : TEXCOORD0;
};

PixelInput VS(VertexInput input)
{
    PixelInput output;
    
    float3 transformedPos = cameraPos.xyz + input.position * scaleFactor.x;
    
    output.screenPos = mul(worldMatrix, float4(transformedPos, 1.0));
    output.screenPos = mul(vp, output.screenPos);
    
    output.screenPos.z = output.screenPos.w;
    
    output.objectSpacePos = input.position;

    return output;
}
