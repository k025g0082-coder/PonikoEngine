#include "resources/Engine/Shader/PostEffect.hlsli"

Texture2D<float4> texture0 : register(t0);
Texture2D<float4> gCompositeTexture : register(t1);
Texture2D<float>  gDepthTexture : register(t2);

SamplerState gSampler : register(s0);


cbuffer BloomBuffer : register(b1)
{
    BloomParam gBloom;
};

cbuffer FogParam : register(b2)
{
    FogParam gFogParam;
};

    struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};


// 深度値から距離フォグ計算を行い、色に適用する
float4 ApplyDistanceFog(FogParam fogParam, float4 color, float depth)
{

    // 背景はフォグを適用しない
    if (depth >= 0.999999f)
    {
        return color;
    }

    // 深度値をカメラからの距離に変換
    const float nearZ = 0.1f;
    const float farZ = 10000.0f;

    float linearDepth =
        (nearZ * farZ) /
        (farZ - depth * (farZ - nearZ));

    // 距離に応じてフォグを強くする
    float fogFactor = saturate(
        (linearDepth - fogParam.fogStart) / (fogParam.fogEnd - fogParam.fogStart)
    );

    color.rgb = lerp(color.rgb, fogParam.fogColor, fogFactor);

    return color;
}

/*
    3D空間Scene用
*/

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float2 uv = input.texcoord; 
    
    // 元となるSceneサンプリング
    float4 color = texture0.Sample(gSampler, uv);
   
    
    /* 
        深度マップFogテクスチャ合成
    */
    if (gFogParam.isFog)
    {
        // 深度マップ
        float depth = gDepthTexture.Sample(gSampler, uv);
        
        /* 線形Fog */
        color = ApplyDistanceFog(gFogParam, color, depth);
    }
    
    
    /* 
        Bloomテクスチャ合成
    */
    float3 bloom = gCompositeTexture.Sample(gSampler, uv).rgb;
    color.rgb += bloom * gBloom.bloomIntensity;
    
    
    output.color = color;
    
    return output;
}