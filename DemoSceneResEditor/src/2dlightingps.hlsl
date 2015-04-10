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

static const float4 lightColor = float4(1, 1, 1, 1);
static const float3 viewVector = float3(0, 0, 1);

float4 main(PSInput input) : SV_TARGET
{
    // Sample material
    float4 diffuseMap = texDiffuse.Sample(samplerState, input.texCoord);
    float4 normalMap = texNormal.Sample(samplerState, input.texCoord);
    float4 materialMap = texMaterial.Sample(samplerState, input.texCoord);

    // Calculate normal
    float3 normal = normalMap.xyz * 2 - 1;

    // Ambient
    diffuseMap.rgb *= normalMap.a;

    // Light intensity
    float3 lightDir = lightPos - float3(input.position.xy, 0);
    lightDir = normalize(lightDir);
    float power = dot(normal, lightDir);

    // Calculate specular
    float3 r = normalize(2 * dot(lightDir, normal) * normal - lightDir);
    float3 v = viewVector;
    float dotProduct = dot(r, v);
    float4 specular = materialMap.r * lightColor * max(pow(dotProduct, materialMap.g * 100), 0) * diffuseMap;

    // Self illumination
    float4 selfIllum = materialMap.b * diffuseMap;
    
    // Final mix
    return saturate(diffuseMap * power * input.color + specular + selfIllum);
}
