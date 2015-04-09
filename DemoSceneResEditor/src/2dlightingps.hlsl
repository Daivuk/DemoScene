cbuffer LightBuffer : register(b0)
{
    float3 lightPos;
    float light_cb_padding;
}

Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
Texture2D texMaterial : register(t2);
SamplerState samplerState : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 diffuseMap = texDiffuse.Sample(samplerState, input.texCoord);
    float4 normalMap = texNormal.Sample(samplerState, input.texCoord);
    float4 materialMap = texMaterial.Sample(samplerState, input.texCoord);

    float3 normal = normalMap.xyz * 2 - 1;

    float3 lightDir = lightPos - float3(input.position.xy, 0);
    lightDir = normalize(lightDir);

    float power = dot(normal, lightDir);

    return diffuseMap * power * input.color;
}
