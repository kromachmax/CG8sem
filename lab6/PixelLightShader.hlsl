struct VSOutput
{
    float4 pos : SV_Position;
};

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
    float4 size;
    float4 color;
};

float4 PS(VSOutput pixel) : SV_Target0
{
    return float4(color.xyz, 0.5);
}
