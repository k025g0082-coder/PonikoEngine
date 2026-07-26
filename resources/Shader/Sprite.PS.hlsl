#include "resources/Shader/Sprite.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// 色
struct Material
{
    float4 color;
};

ConstantBuffer<Material> material : register(b0); // rootParametorから読み取る場所

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float4 texColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 色を乗算
    output.color = material.color * texColor;
    
    return output;
}