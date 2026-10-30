
// PS INPUTの読込
#include "resources/Engine/Shader/PostEffect.hlsli"

// シーンの描画結果
Texture2D<float4> gTexture0 : register(t0);

SamplerState gSampler : register(s0);

///* ブルームのパラメータ */
cbuffer BloomBuffer : register(b1)
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
    uint width, height;
    gTexture0.GetDimensions(width, height);

    float2 texelSize = float2(
    1.0f / width,
    1.0f / height
);
    
    float4 color = 0.0f;

    color += gTexture0.Sample(gSampler, input.texcoord) * 0.4f;
    color += gTexture0.Sample(
        gSampler, input.texcoord + float2(texelSize.x * gBloom.blurTexelSize, 0)) * 0.15f;
    color += gTexture0.Sample(
        gSampler, input.texcoord - float2(texelSize.x * gBloom.blurTexelSize, 0)) * 0.15f;
    color += gTexture0.Sample(
        gSampler, input.texcoord + float2(0, texelSize.y * gBloom.blurTexelSize)) * 0.15f;
    color += gTexture0.Sample(
        gSampler, input.texcoord - float2(0, texelSize.y * gBloom.blurTexelSize)) * 0.15f;

    return color;

}

/* ピラミッド方式 */
float4 DownBlurSample(VertexShaderOutput input) : SV_TARGET
{
    
    // 入力テクスチャのサイズを取得
        uint width, height;
        gTexture0.GetDimensions(width, height);

    // 1ピクセル分のUVサイズ
        float2 texelSize = 1.0f / float2(width, height);

        float2 uv = input.texcoord;

    // 13-tap sampling
        float3 a = gTexture0.Sample(gSampler, uv + float2(-2, 2) * texelSize).rgb;
        float3 b = gTexture0.Sample(gSampler, uv + float2(0, 2) * texelSize).rgb;
        float3 c = gTexture0.Sample(gSampler, uv + float2(2, 2) * texelSize).rgb;

        float3 d = gTexture0.Sample(gSampler, uv + float2(-2, 0) * texelSize).rgb;
        float3 e = gTexture0.Sample(gSampler, uv).rgb;
        float3 f = gTexture0.Sample(gSampler, uv + float2(2, 0) * texelSize).rgb;

        float3 g = gTexture0.Sample(gSampler, uv + float2(-2, -2) * texelSize).rgb;
        float3 h = gTexture0.Sample(gSampler, uv + float2(0, -2) * texelSize).rgb;
        float3 i = gTexture0.Sample(gSampler, uv + float2(2, -2) * texelSize).rgb;

        float3 j = gTexture0.Sample(gSampler, uv + float2(-1, 1) * texelSize).rgb;
        float3 k = gTexture0.Sample(gSampler, uv + float2(1, 1) * texelSize).rgb;
        float3 l = gTexture0.Sample(gSampler, uv + float2(-1, -1) * texelSize).rgb;
        float3 m = gTexture0.Sample(gSampler, uv + float2(1, -1) * texelSize).rgb;

    // 重み付け合成
        float3 downsample = e * 0.125;

        downsample += (a + c + g + i) * 0.03125;
        downsample += (b + d + f + h) * 0.0625;
        downsample += (j + k + l + m) * 0.125;

        return float4(downsample, 1.0f);
}

// UpSample ただサンプルするだけ♪
float4 UpBlurSample(VertexShaderOutput input) : SV_TARGET
{
    float3 color = gTexture0.Sample(gSampler, input.texcoord).rgb;
    return float4(color, 1.0f);
}