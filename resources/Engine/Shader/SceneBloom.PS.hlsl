
// PS INPUTの読込
#include "resources/Engine/Shader/PostEffect.hlsli"

// シーンの描画結果
Texture2D<float4> gTexture0 : register(t0);

SamplerState gSampler : register(s0);

///* ブルームのパラメータ */
cbuffer BloomBuffer : register(b2)
{
    BloomParam gBloom;
};

/* シーン上の明るいところだけを摘出 */
float4 SceneBloom(VertexShaderOutput input) : SV_TARGET {
     
    float2 uv = input.texcoord;
    
    float3 color = gTexture0.Sample(gSampler, uv).rgb;

    // 明るい部分取り出す
    float luminance = dot(
        color,
        float3(0.2126f, 0.7152f, 0.0722f)
    );

    float3 bloomColor = color * smoothstep(
    gBloom.bloomThreshold,
    gBloom.bloomThreshold + gBloom.bloomSoftness,
    luminance);

    return float4(bloomColor, 1.0f);
}


/*  ぼかし　（簡易版）*/ 
float4 SceneBlur(VertexShaderOutput input) : SV_TARGET
{
    float2 texelSize = 1.0f / float2(1280.0f, 720.0f);

    float4 color = 0.0f;

    color += gTexture0.Sample(gSampler, input.texcoord) * 0.4f;
    color += gTexture0.Sample(
        gSampler, input.texcoord + float2(texelSize.x, 0)) * 0.15f;
    color += gTexture0.Sample(
        gSampler, input.texcoord - float2(texelSize.x, 0)) * 0.15f;
    color += gTexture0.Sample(
        gSampler, input.texcoord + float2(0, texelSize.y)) * 0.15f;
    color += gTexture0.Sample(
        gSampler, input.texcoord - float2(0, texelSize.y)) * 0.15f;

    return color;
}


