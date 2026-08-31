#include "resources/Engine/Shader/Particle.hlsli"

Texture2D<float4> texture0 : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float4 color;
    int enableLighting;
    float3 padding;
    float4x4 uvTransform;
};

ConstantBuffer<Material> material : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float4 uvTransform = mul(float4(input.texcoord, 0.0f, 1.0f), material.uvTransform);
    float2 uv = uvTransform.xy;
    
    float4 textureColor = texture0.Sample(gSampler, uv);
    
    output.color = material.color * input.color * textureColor;

    if (output.color.a == 0.0)
    {
        discard;
    }
    
        return output;
}