#include "object3d.hlsli"

Texture2D<float4> texture0 : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float4 color;
    int enableLighting;
    float3 padding;
    float4x4 uvTransform;
};

struct DirectionalLight
{
    float4 color; //ライト色
    float3 direction; //ライトの向き
    float intensity; //ライトの強さ
};

ConstantBuffer<Material> material : register(b0);
ConstantBuffer<DirectionalLight> directionalLight : register(b1);

struct PixelShaderOutput {
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;
    
   // float4 texColor = texture0.Sample(gSampler, input.texcoord);
  
   // output.color = material.color * texColor;
    
    float4 uvTransform = mul(float4(input.texcoord, 0.0f, 1.0f), material.uvTransform);
    float2 uv = uvTransform.xy;
    
    float4 textureColor = texture0.Sample(gSampler, uv);
    
    if (material.enableLighting != 0)
    
    { //Lightingする場合
        //float cos = saturate(dot(normalize(input.normal), -directionalLight.direction));
        
        float NdotL = dot(normalize(input.normal), -directionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        output.color = material.color * textureColor * directionalLight.color * cos * directionalLight.intensity;
    }
    else
    { //Lightingしない場合。前回までと同じ演算
        output.color = material.color * textureColor;
    }

    return output;
}