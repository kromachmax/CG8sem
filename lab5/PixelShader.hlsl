Texture2D colorTexture : register(t0);

SamplerState colorSampler : register(s0);

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

float4 PS(VSOutput pixel) : SV_Target0
{
    return float4(colorTexture.Sample(colorSampler, pixel.uv).xyz, 1.0);
}
