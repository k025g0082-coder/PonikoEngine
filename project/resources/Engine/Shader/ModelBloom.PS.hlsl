
#include "resources/Engine/Shader/ModelBloom.hlsli"
#include "resources/Engine/Shader/PostEffect.hlsli"

Texture2D<float4> texture0 : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float4 color;
    int enableLighting;
    float3 padding;
    float4x4 uvTransform;
};

// モデルのマテリアル
ConstantBuffer<Material> material : register(b0);

///* ブルームのパラメータ */
cbuffer BloomBuffer : register(b1)
{
    BloomParam gBloom;
};


struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VeltexShaderOutput input)
{
    PixelShaderOutput output;
    
    /* UV変換 */
    float4 uvTransform = mul(float4(input.texcoord, 0.0f, 1.0f), material.uvTransform);
    float2 uv = uvTransform.xy;
    
    float4 textureColor = texture0.Sample(gSampler, uv);
    
    float4 color = texture0.Sample(gSampler, uv);
    
    // マテリアルのカラー適用
    color *= material.color;
    
    // 透明部分を除外
    if (color.a <= 0.01f)
    {
        discard;
    }

    
    // 明るい部分取り出す
    float luminance = dot(
        color.rgb,
        float3(0.2126f, 0.7152f, 0.0722f)
    );

    float3 bloomColor = color.rgb * smoothstep(
    gBloom.bloomThreshold,
    gBloom.bloomThreshold + gBloom.bloomSoftness,
    luminance);

    
    
    output.color = float4(bloomColor, 1.0f);

    return output;
}