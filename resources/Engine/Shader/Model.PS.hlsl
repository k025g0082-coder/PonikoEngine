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

struct PointLightParam
{

    float4 color; // ライト色
    float3 position; // ライトの位置
    float intensity; // ライトの強さ

    float radius; // 光が届く範囲
    float3 padding;

};

struct SpotLightParam
{

    float4 color; // ライト色
    float3 position; // ライトの位置
    float intensity; // ライトの強さ

    float3 direction; // ライトの向き
    float angle; // 光の広がる角度

    float distance; // 光が届く距離
    float falloff; // 距離による減衰

};


ConstantBuffer<Material> material : register(b0);

ConstantBuffer<DirectionalLightParam> directionalLight : register(b1);
ConstantBuffer<SpotLightParam> spotLight : register(b2);
ConstantBuffer<PointLightParam> pointLight : register(b3);

struct PixelShaderOutput {
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
    
    PixelShaderOutput output;
    
    float4 uvTransform = mul(float4(input.texcoord, 0.0f, 1.0f), material.uvTransform);
    float2 uv = uvTransform.xy;
    
    float4 textureColor = texture0.Sample(gSampler, uv);
    
    //Lightingする場合
    if (material.enableLighting != 0)
    {
        float3 normal = normalize(input.normal);

    // =========================
    // Directional Light
    // =========================
        float NdotL = dot(
        normal,
        -directionalLight.direction
    );

        float directionalCos =
        pow(saturate(NdotL * 0.5f + 0.5f), 2.0f);

        float3 lighting =
        directionalLight.color.rgb *
        directionalCos *
        directionalLight.intensity;


    // =========================
    // Point Light
    // =========================
        float3 pointDirection =
    normalize(pointLight.position - input.worldPosition);

        float pointNdotL =
    saturate(dot(normal, -pointDirection));

        float pointDistance =
    distance(pointLight.position, input.worldPosition);

        float pointAttenuation =
    saturate(1.0f - pointDistance / pointLight.radius);

        lighting +=
    pointLight.color.rgb *
    pointNdotL *
    pointAttenuation *
    pointLight.intensity;

    // =========================
    // Spot Light
    // =========================
        float3 spotDirection =
        normalize(spotLight.position - input.worldPosition);

        float spotNdotL =
        saturate(dot(normal, spotDirection));

        float spotDistance =
        distance(spotLight.position, input.worldPosition);

    // 距離による減衰
        float distanceAttenuation =
        saturate(1.0f - spotDistance / spotLight.distance);

    // スポットライトの向き
        float spotCos =
        dot(
            normalize(-spotLight.direction),
            spotDirection
        );

    // 光の範囲
        float spotFactor =
        saturate(
            (spotCos - cos(spotLight.angle))
        );

        lighting +=
        spotLight.color.rgb *
        spotNdotL *
        distanceAttenuation *
        spotFactor *
        spotLight.intensity;


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