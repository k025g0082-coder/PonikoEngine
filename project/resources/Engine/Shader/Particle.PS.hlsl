#include "resources/Engine/Shader/Particle.hlsli"

Texture2D<float4> gTextures[] : register(t1);
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

    float4 uvTransform =
        mul(
            float4(input.texcoord, 0.0f, 1.0f),
            material.uvTransform
        );

    float2 uv = uvTransform.xy;

    // ★ インスタンスごとのテクスチャを取得
    float4 textureColor =
        gTextures[input.textureIndex].Sample(gSampler, uv);

    output.color =
        material.color *
        input.color *
        textureColor;

    if (output.color.a == 0.0f)
    {
        discard;
    }

    return output;
}