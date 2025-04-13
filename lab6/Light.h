struct Light
{
    float4 pos;
    float4 color;
};

cbuffer SceneBuffer : register (b0)
{
    float4x4 vp;
    float4 cameraPos;
    int4 lightCount;
    Light lights[10];
    float4 ambientColor;
};

float3 CalculateColor(float3 objColor, float3 objNormal, float3 pos, float shine, bool trans)
{
    float3 finalColor = float3(0, 0, 0);

    for (int i = 0; i < lightCount.x; i++)
    {
        float3 normal = objNormal;

        float3 lightDir = lights[i].pos.xyz - pos;
        float lightDist = length(lightDir);
        lightDir /= lightDist;

        float atten = clamp(1.0 / (lightDist * lightDist), 0, 1);

        if (trans && dot(lightDir, objNormal) < 0.0)
        {
            normal = -normal;
        }

        finalColor += objColor * max(dot(lightDir, normal), 0) * atten * lights[i].color.xyz;

        float3 viewDir = normalize(cameraPos.xyz - pos);
        float3 reflectDir = reflect(-lightDir, normal);

        float spec = shine > 0 ? pow(max(dot(viewDir, reflectDir), 0.0), shine) : 0.0;

        finalColor += objColor * 0.5 * spec * lights[i].color.xyz;
    }

    return finalColor;
}