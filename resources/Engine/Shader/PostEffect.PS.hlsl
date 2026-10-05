#include "resources/Engine/Shader/PostEffect.hlsli"

Texture2D<float4> texture0 : register(t0);
SamplerState gSampler : register(s0);

struct PostEffectParam
{
    float blurPower;
    float time; // 時間
    
    float hzWavePower; //  横ラスターの強さ
    float vtWavePower; // 縦ラスターの強さ
    
    float monotone; // モノクロ化
    float colorInversion; // 色反転

    float noise; // ノイズ
    float neon; // ネオン
    
    float fishLens; // 魚眼レンズ
    
    float glitch; // グリッチ
    
    float4 color;
};

cbuffer PostEffect : register(b1)
{
    PostEffectParam gPostEffect;
};

// これやったら怒られた　命令数が多すぎるみたいな
float4 Get5x5GaussianBlur(Texture2D<float4> tex, SamplerState smp, float2 uv, float dx, float dy, float4 rect)
{
    float4 ret = tex.Sample(smp, uv);
    float4 blurColor = float4(0, 0, 0, 0);
    
    float weights[5][5] =
    {
        { 1 / 273.0, 4 / 273.0, 7 / 273.0, 4 / 273.0, 1 / 273.0 },
        { 4 / 273.0, 16 / 273.0, 26 / 273.0, 16 / 273.0, 4 / 273.0 },
        { 7 / 273.0, 26 / 273.0, 41 / 273.0, 26 / 273.0, 7 / 273.0 },
        { 4 / 273.0, 16 / 273.0, 26 / 273.0, 16 / 273.0, 4 / 273.0 },
        { 1 / 273.0, 4 / 273.0, 7 / 273.0, 4 / 273.0, 1 / 273.0 }
    };
    
    float offsets[5] = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            float2 offset = float2(offsets[i] * dx, offsets[j] * dy);
            float2 sampleUV = uv + offset;
            sampleUV.x = clamp(sampleUV.x, rect.x + dx * 0.5, rect.z - dx * 0.5);
            sampleUV.y = clamp(sampleUV.y, rect.y + dy * 0.5, rect.w - dy * 0.5);
            blurColor += tex.Sample(smp, sampleUV) * weights[i][j];
        }
    }
    return float4(blurColor.rgb, ret.a);
}

// 疑似乱数　シェーダー限定
float rand(float x)
{
    return frac(sin(x) * 43758.5453f);
}

float randRange(float x, float minValue, float maxValue)
{
    return lerp(minValue, maxValue, rand(x));
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{

    PixelShaderOutput output;
    
    float2 uv = input.texcoord;
    float4 color = texture0.Sample(gSampler, uv);
    
    //　魚眼レンズ
    
    // 距離
    float2 centerOffset = uv - 0.5f;

    float dist = length(centerOffset);

    float2 fishUV = 0.5f + centerOffset * (1.0f + dist * gPostEffect.fishLens);
    
    // 範囲外を黒く塗りつぶすか
    if (any(fishUV < 0.0) || any(fishUV > 1.0))
    {
        output.color = float4(0, 0, 0, 1);
        return output;
    }
    else
    {
        uv = fishUV;
     
    }
    

    // ===
   
    
    // ==== 波なみにする処理
    
    float waveX = sin(uv.y * 20.0f + gPostEffect.time * 5.0f) * 0.01f * gPostEffect.hzWavePower;
    float waveY = sin(uv.x * 20.0f + gPostEffect.time * 4.5f) * gPostEffect.vtWavePower * 0.05f;

    uv.x += waveX; // 横方向に波打たせる
    uv.y += waveY; // 横方向に波打たせる
    
    // ====
    
    // === グリッチ 
    
    // 時間を0.2秒で割ることで一定間隔の乱数を作る
    float interval = 0.075f;
    float tmpSeed = floor(gPostEffect.time / interval);
    
    float divisionsX = 1.0f;
    float divisionsY = 5.0f + randRange(tmpSeed, 0, 30.0f);
   
    // 座標がどこの帯に属しているのか
    float2 blockPos = floor(uv * float2(divisionsX, divisionsY));
    
    // float2の中身は疑似乱数をつくるためなので、何でもいい
    float blockNoise = rand(dot(blockPos, float2(12.9898f, 78.233f)) + tmpSeed);

    float move = (blockNoise - 0.5) * 0.1;
    
    // 一定確率
    if (blockNoise > 0.80f)
    {
        // float2の中身は疑似乱数をつくるためなので、何でもいい
        float dir = rand(dot(blockPos, float2(31.7, 17.3)) + tmpSeed + 100.0f);

        // ずらす幅
        float move = randRange(tmpSeed, 0.01f, 0.1f);

        if (dir > 0.5f)
        {
            move *= -1.0f;
        }

        uv.x += move;
    }
   
   
    
    // ====
    
    // ==== ぼかし
    
    float offset = gPostEffect.blurPower * 0.0018f;
    
    color = float4(0, 0, 0, 0);
    
    // 3x3簡易ガウシアン（9サンプル）
    color += texture0.Sample(gSampler, uv + float2(-offset, -offset));
    color += texture0.Sample(gSampler, uv + float2(0, -offset));
    color += texture0.Sample(gSampler, uv + float2(offset, -offset));
    color += texture0.Sample(gSampler, uv + float2(-offset, 0));
    color += texture0.Sample(gSampler, uv + float2(offset, 0));
    color += texture0.Sample(gSampler, uv + float2(-offset, offset));
    color += texture0.Sample(gSampler, uv + float2(0, offset));
    color += texture0.Sample(gSampler, uv + float2(offset, offset));

    color /= 9.0f; // 9で割る

    // ====
    
     // === ネオン
    
    // ずらしてランプリング
    float2 neonOffset = float2(0.01f, 0.0f);

    float4 neonSample = texture0.Sample(gSampler, uv + neonOffset);
    
    float brightness =
    dot(neonSample.rgb, float3(0.299f, 0.587f, 0.114f));

    float mask = step(0.7f, brightness);

    // これもいいからのこしとく
    //float3 glow = 0;

    //glow += texture0.Sample(gSampler, uv + float2(0.005f, 0)).rgb;
    //glow += texture0.Sample(gSampler, uv + float2(0.010f, 0)).rgb;
    //glow += texture0.Sample(gSampler, uv + float2(0.015f, 0)).rgb;

    //glow /= 3.0f;

    //color.rgb += glow * gPostEffect.neon;
   
    // 要はそれぞれずらしたのを加算？
    
    float r = texture0.Sample(gSampler, uv + float2(0.01f * gPostEffect.neon, 0)).r;
    float g = texture0.Sample(gSampler, uv).g;
    float b = texture0.Sample(gSampler, uv - float2(0.01f * gPostEffect.neon, 0)).b;

    color.rgb = float3(r, g, b);
    
    // ===
    
    // モノトーン
    
    float3 factor = { 0.299f, 0.587f, 0.114f };

    float gray = dot(color.rgb, factor);

    color.rgb = lerp(color.rgb, gray.xxx, gPostEffect.monotone);
    
    // ===
    
    // 色反転
   
    float4 invertedColor = 1.0f - color;

    // 補完する（なるほど）
    color = lerp(color, invertedColor, gPostEffect.colorInversion);
    
    // ===
    
    //　ノイズ
    float resolution = 1.0f; //解像度
    float noiseScale = 1.0f; // ノイズの大きさ
    float noiseSpeed = 1.0f; // ノイズのhayasa
   
    float2 scaledUV = uv * resolution * noiseScale;
    float2 timeOffset = float2(gPostEffect.time * noiseSpeed, gPostEffect.time * 1.7);
    
    //RGBそれぞれ少しずらしたノイズ
    float nR = frac(sin(dot(scaledUV + timeOffset.xy, float2(12.9898, 78.233))) * 43758.5453f);
    float nG = frac(sin(dot(scaledUV + timeOffset.xy + 3.0, float2(12.9898, 78.233))) * 43758.5453f);
    float nB = frac(sin(dot(scaledUV + timeOffset.xy + 6.0, float2(12.9898, 78.233))) * 43758.5453f);
    
    // 補完
    color.rgb = lerp(
    color.rgb,
    float3(nR, nG, nB),
    gPostEffect.noise
);
    
    // ===
    

    // ここで色を乗算！
    float4 finalColor = color * gPostEffect.color;
    
// ==== 走査線
    
    float y = uv.y * 720.0f;

    if (fmod(y, 2.0f) < 1.0f)
    {
        finalColor.rgb *= 0.7f; // 30%暗くする
    }
// ===
    
    output.color = finalColor;

    
    
    return output;
}