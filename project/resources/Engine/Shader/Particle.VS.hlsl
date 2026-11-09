
#include "resources/Engine/Shader/Particle.hlsli"

struct ParticleData
{
    float4x4 WVP;
    float4x4 World;
    float4 color;
    uint textureIndex;
};

StructuredBuffer<ParticleData> gParticles : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
};

//struct VertexShaderOutput
//{
//    float4 position : SV_POSITION;
//    float2 texcoord : TEXCOORD0;
//    float3 normal : NORMAL0;
    
//    float4 color : COLOR0;
//};



VertexShaderOutput main(VertexShaderInput input,uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gParticles[instanceId].WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) gParticles[instanceId].World));
    
    // インスタンスごとの色
    output.color = gParticles[instanceId].color;
    
    // インスタンスごとのテクスチャ番号
    output.textureIndex =
        gParticles[instanceId].textureIndex;

    return output;
}
