
#include "resources/Engine/Shader/Particle.hlsli"

struct ParticleData
{
    float4x4 WVP;
    float4x4 World;
    float4 color;
};

StructuredBuffer<ParticleData> gParticles : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
};

VertexShaderOutput main(VertexShaderInput input,uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gParticles[instanceId].WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) gParticles[instanceId].World));
    
    // インスタンスごとの色
    output.color = gParticles[instanceId].color;
    
    return output;
}
