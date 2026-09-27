#include "resources/Engine/Shader/Model.hlsli"

Texture2D<float4> texture0 : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float4 color;
    int enableLighting;
    float3 padding;
    float4x4 uvTransform;
};

struct DirectionalLightParam
{
    float4 color; //ライト色
    float3 direction; //ライトの向き
    float intensity; //ライトの強さ
};

struct DirectionalLightsParam
{
    DirectionalLightParam lights[10];
    int count;
    
    float3 padding;
};



struct PointLightParam
{

    float4 color; // ライト色
    float3 position; // ライトの位置
    float intensity; // ライトの強さ

    float radius; // 光が届く範囲
    float3 padding;

};

struct PointLightsParam
{
    PointLightParam lights[10];
    int count;
    
    float3 padding;
};



struct SpotLightParam
{

    float4 color; // ライト色
    float3 position; // ライトの位置
    float intensity; // ライトの強さ

    float3 direction; // ライトの向き
    
    float innerAngle; // この角度以内なら最大の明るさ
    float outerAngle; // これを超えたら光が0になる

    float distance; // 光が届く距離
    float falloff; // 距離による減衰
    float padding;

};

struct SpotLightsParam
{
    SpotLightParam lights[10];
    int count;
    
    float3 padding;
};


ConstantBuffer<Material> material : register(b0);

ConstantBuffer<DirectionalLightsParam> directionalLights : register(b1);
ConstantBuffer<SpotLightsParam> spotLights : register(b2);
ConstantBuffer<PointLightsParam> pointLights : register(b3);


struct PixelShaderOutput {
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
    
    PixelShaderOutput output;
    
    float4 uvTransform = mul(float4(input.texcoord, 0.0f, 1.0f), material.uvTransform);
    float2 uv = uvTransform.xy;
    
    float4 textureColor = texture0.Sample(gSampler, uv);
   
    
    if (textureColor.a <= 0.1f)
    {
       discard;
    }
    
    
    //Lightingする場合
    if (material.enableLighting != 0)
    {
        float3 normal = normalize(input.normal);
        
        float3 lighting = float3(0.0f, 0.0f, 0.0f);
    
      
        
    // =========================
    // Directional Light
    // =========================
     
        for (int i = 0; i < directionalLights.count; i++)
        {
                float NdotL = dot(
            normal,
            -directionalLights.lights[i].direction
            );

                float directionalCos =
            pow(saturate(NdotL * 0.5f + 0.5f), 2.0f);

                lighting +=
            directionalLights.lights[i].color.rgb *
            directionalCos *
            directionalLights.lights[i].intensity;
        }

    // =========================
    // Point Light
    // =========================

        for (int i = 0; i < pointLights.count; i++)
        {
                float3 pointDirection =
            normalize(pointLights.lights[i].position - input.worldPosition);

                float pointNdotL =
            saturate(dot(normal, -pointDirection));

                float pointDistance =
            distance(pointLights.lights[i].position, input.worldPosition);

                float pointAttenuation =
            saturate(
                1.0f -
                pointDistance / pointLights.lights[i].radius
            );

                lighting +=
            pointLights.lights[i].color.rgb *
            pointNdotL *
            pointAttenuation *
            pointLights.lights[i].intensity;
        }

    // =========================
    // Spot Light
    // =========================
        
        for (int i = 0; i < spotLights.count; i++)
        {
    // 表面 → ライト（法線ドット用）
            float3 toLight = normalize(spotLights.lights[i].position - input.worldPosition);
            float NdotL = saturate(dot(normal, toLight));

    // 距離減衰
            float dist = distance(spotLights.lights[i].position, input.worldPosition);
            float distAtten = saturate(1.0f - dist / spotLights.lights[i].distance);
            distAtten *= distAtten; // 見た目を良くするなら二乗

    // 角度減衰（ここが重要）
            float3 lightToSurface = -toLight; // ライト → 表面
            float3 lightDir = normalize(spotLights.lights[i].direction);
            float spotCos = dot(lightDir, lightToSurface);

            float innerCos = cos(radians(spotLights.lights[i].innerAngle));
            float outerCos = cos(radians(spotLights.lights[i].outerAngle));

            float angleRange = max(innerCos - outerCos, 1e-4f); // ゼロ除算防止
            float spotFactor = saturate((spotCos - outerCos) / angleRange);

            lighting += spotLights.lights[i].color.rgb
              * NdotL
              * distAtten
              * spotFactor
              * spotLights.lights[i].intensity;
        }
            output.color =
        material.color *
        textureColor *
        float4(lighting, 1.0f);
        
    }
    else
    {
        // 前回と同じ
        output.color = material.color * textureColor;
    }
    
    return output;
}