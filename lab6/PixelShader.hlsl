#include "Light.h"

cbuffer MaterialProperties : register(b1)
{
    float4x4 worldTransform;
    float4x4 normalMatrix;
    float4 glossiness;
};

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
SamplerState textureSampler : register(s0);

struct PixelInput
{
    float4 clipPos : SV_Position;
    float4 globalPos : POSITION;
    float3 tangentVector : TANGENT;
    float3 normalVector : NORMAL;
    float2 texCoords : TEXCOORD;
};

float4 PS(PixelInput input) : SV_Target0
{
    float3 baseColor = diffuseMap.Sample(textureSampler, input.texCoords).rgb;

    float3 computedNormal = float3(0.0, 0.0, 0.0);

    float3 biNormal = normalize(cross(input.normalVector, input.tangentVector));

    float3 sampledNormal = normalMap.Sample(textureSampler, input.texCoords).rgb;
    float3 adjustedNormal = sampledNormal * 2.0 - 1.0;

    computedNormal = adjustedNormal.x * normalize(input.tangentVector) +
                     adjustedNormal.y * biNormal +
                     adjustedNormal.z * normalize(input.normalVector);

    float3 finalColor = CalculateColor(baseColor, computedNormal, input.globalPos.xyz, glossiness.x, false);
    return float4(finalColor, 1.0);
}