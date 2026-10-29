#include "resources/Engine/Shader/PostEffect.hlsli"

Texture2D<float4> texture0 : register(t0);
Texture2D<float4> gCompositeTexture : register(t1);

SamplerState gSampler : register(s0);

cbuffer PostEffect : register(b1)
{
    PostEffectParam gPostEffect;
};

cbuffer BloomBuffer : register(b2)
{
    BloomParam gBloom;
};


struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};


// 乱数


float rand(float x)
{
    return frac(sin(x) * 43758.5453f);
}

float randRange(float x, float minValue, float maxValue)
{
    return lerp(minValue, maxValue, rand(x));
}


/* シーンの描画結果と合成するテクスチャを合成します */
float4 SceneComposite(VertexShaderOutput input) : SV_TARGET
{
    float4 scene = texture0.Sample(gSampler, input.texcoord);
    float3 bloom = gCompositeTexture.Sample(gSampler, input.texcoord).rgb;

    return float4(scene.rgb + bloom * gBloom.bloomIntensity, scene.a);
}


// 魚眼レンズ


// 範囲外なら黒く塗りつぶすかどうかも指定
float2 fishLens(float2 uv, float power, bool fillBlack, out bool isOutside)
{
    float2 centerOffset = uv - 0.5f;
    float dist = length(centerOffset);

    float2 fishUV = 0.5f + centerOffset * (1.0f + dist * power);

    isOutside = any(fishUV < 0.0f) || any(fishUV > 1.0f);

    if (isOutside && fillBlack)
    {
        return float2(-1.0f, -1.0f);
    }

    return isOutside ? uv : fishUV;
}


// 波
float2 ApplyWave(float2 uv)
{
    float waveX = sin(uv.y * 20.0f + gPostEffect.time * 5.0f)
        * 0.01f * gPostEffect.hzWavePower;

    float waveY = sin(uv.x * 20.0f + gPostEffect.time * 4.5f)
        * gPostEffect.vtWavePower * 0.05f;

    uv.x += waveX;
    uv.y += waveY;

    return uv;
}


// グリッチ
float2 ApplyGlitch(float2 uv)
{
    float interval = 0.075f;
    float tmpSeed = floor(gPostEffect.time / interval);

    float divisionsX = 1.0f;
    float divisionsY = 5.0f + randRange(tmpSeed, 0.0f, 30.0f);

    float2 blockPos = floor(uv * float2(divisionsX, divisionsY));

    float blockNoise = rand(
        dot(blockPos, float2(12.9898f, 78.233f)) + tmpSeed
    );

    if (blockNoise > 0.80f)
    {
        float dir = rand(
            dot(blockPos, float2(31.7f, 17.3f)) + tmpSeed + 100.0f
        );

        float move = randRange(tmpSeed, 0.01f, 0.1f);

        if (dir > 0.5f)
        {
            move *= -1.0f;
        }

        uv.x += move * gPostEffect.glitch;
    }

    return uv;
}


// ぼかし
float4 ApplyBlur(float2 uv)
{
    float offset = gPostEffect.blurPower * 0.0018f;

    float4 color = float4(0, 0, 0, 0);

    color += texture0.Sample(gSampler, uv + float2(-offset, -offset));
    color += texture0.Sample(gSampler, uv + float2(0, -offset));
    color += texture0.Sample(gSampler, uv + float2(offset, -offset));

    color += texture0.Sample(gSampler, uv + float2(-offset, 0));
    color += texture0.Sample(gSampler, uv);
    color += texture0.Sample(gSampler, uv + float2(offset, 0));

    color += texture0.Sample(gSampler, uv + float2(-offset, offset));
    color += texture0.Sample(gSampler, uv + float2(0, offset));
    color += texture0.Sample(gSampler, uv + float2(offset, offset));

    return color / 9.0f;
}


// ネオン・RGBずらし
float4 ApplyNeon(float2 uv, float4 color)
{
    float r = texture0.Sample(
        gSampler, uv + float2(0.01f * gPostEffect.neon, 0)
    ).r;

    float g = texture0.Sample(gSampler, uv).g;

    float b = texture0.Sample(
        gSampler, uv - float2(0.01f * gPostEffect.neon, 0)
    ).b;

    color.rgb = float3(r, g, b);

    return color;
}


// モノクロ
float4 ApplyMonotone(float4 color)
{
    float3 factor = float3(0.299f, 0.587f, 0.114f);

    float gray = dot(color.rgb, factor);

    color.rgb = lerp(
        color.rgb,
        gray.xxx,
        gPostEffect.monotone
    );

    return color;
}


// 色反転
float4 ApplyColorInversion(float4 color)
{
    float4 invertedColor = 1.0f - color;

    return lerp(
        color,
        invertedColor,
        gPostEffect.colorInversion
    );
}


// ノイズ
float4 ApplyNoise(float2 uv, float4 color)
{
    float resolution = 1.0f;
    float noiseScale = 1.0f;
    float noiseSpeed = 1.0f;

    float2 scaledUV = uv * resolution * noiseScale;

    float2 timeOffset = float2(
        gPostEffect.time * noiseSpeed,
        gPostEffect.time * 1.7f
    );

    float nR = frac(
        sin(dot(scaledUV + timeOffset.xy, float2(12.9898f, 78.233f)))
        * 43758.5453f
    );

    float nG = frac(
        sin(dot(scaledUV + timeOffset.xy + 3.0f, float2(12.9898f, 78.233f)))
        * 43758.5453f
    );

    float nB = frac(
        sin(dot(scaledUV + timeOffset.xy + 6.0f, float2(12.9898f, 78.233f)))
        * 43758.5453f
    );

    color.rgb = lerp(
        color.rgb,
        float3(nR, nG, nB),
        gPostEffect.noise
    );

    return color;
}


// 色乗算
float4 ApplyColor(float4 color)
{
    return color * gPostEffect.color;
}


// 走査線
float4 ApplyScanLine(float2 uv, float4 color)
{
    float y = uv.y * 720.0f;

    if (fmod(y, 2.0f) < 1.0f)
    {
        color.rgb *= 1.0f - gPostEffect.scanLine;
    }

    return color;
}


// メイン
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float2 uv = input.texcoord; 

    // シーン画像
    float4 color = texture0.Sample(gSampler, uv);
    
    // =========-こっからポストエフェクト

    
    // 魚眼レンズ
    bool isOutside = false;

    uv = fishLens(
        uv,
        gPostEffect.fishLens,
        true,
        isOutside
    );

    // 魚眼で範囲外になったら黒くする
    if (isOutside)
    {
        output.color = float4(0, 0, 0, 1);
        return output;
    }

    // 波
    uv = ApplyWave(uv);

    // グリッチ
    uv = ApplyGlitch(uv);

    // ぼかし
    color = ApplyBlur(uv);

    // ネオン
    color = ApplyNeon(uv, color);

    // モノクロ
    color = ApplyMonotone(color);

    // 色反転
    color = ApplyColorInversion(color);

    // ノイズ
    color = ApplyNoise(uv, color);

    // 色乗算
    color = ApplyColor(color);

    // 走査線
    color = ApplyScanLine(uv, color);
    
    // 最後にBloomを加算
    float3 bloom = gCompositeTexture.Sample(gSampler, uv).rgb;
    color.rgb += bloom * gBloom.bloomIntensity;
    
    
    output.color = color;
    
    return output;
}