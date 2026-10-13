
#include "resources/Engine/Shader/Model.hlsli"

// WVP
struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
   
};

// アニメ有り無し
struct ModelSkinningParam
{
    int isSkinning;
    float3 padding;
   
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register( b0 );
ConstantBuffer<ModelSkinningParam> gModelSkinningParam : register( b5 ); // b数字はﾊﾟｲﾌﾟﾗｲﾝの数字と一致させるように

// なんでかっこ？ｗ
cbuffer Skinning : register(b4)
{
    float4x4 gBoneMatrices[128];
};


struct ShadowTransform
{
    float4x4 lightWVP;
};

ConstantBuffer<ShadowTransform> gShadowTransform : register(b6);


struct VertexShaderInput
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
  
    uint4  boneIndex : BLENDINDICES;
    float4 weight : BLENDWEIGHT;
};

struct VeltexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    
    float3 worldPosition : POSITION0;
    
    // 追加：ライト空間のクリップ座標
    float4 lightPosition : TEXCOORD1;
};


VeltexShaderOutput main(VertexShaderInput input)
{
    VeltexShaderOutput output;

    float4 localPosition;
    float3 localNormal;

    // アニメあり
    if (gModelSkinningParam.isSkinning != 0)
    {
        float4 skinnedPosition = float4(0, 0, 0, 0);
        float3 skinnedNormal = float3(0, 0, 0);

        for (int i = 0; i < 4; i++)
        {
            skinnedPosition +=
                mul(
                    input.position,
                    gBoneMatrices[input.boneIndex[i]]
                ) * input.weight[i];

            skinnedNormal +=
                mul(
                    input.normal,
                    (float3x3) gBoneMatrices[input.boneIndex[i]]
                ) * input.weight[i];
        }

        localPosition = skinnedPosition;
        localNormal = skinnedNormal;
    }
    else
    {
        localPosition = input.position;
        localNormal = input.normal;
    }

    // 画面に描画する座標
    output.position =
        mul(localPosition, gTransformationMatrix.WVP);

    output.texcoord = input.texcoord;

    // ワールド空間の法線
    output.normal =
        normalize(
            mul(
                localNormal,
                (float3x3) gTransformationMatrix.World
            )
        );

    // ワールド空間の頂点座標
    output.worldPosition =
        mul(localPosition, gTransformationMatrix.World).xyz;
    
    float4 worldPosition =
    mul(localPosition, gTransformationMatrix.World);// 上のと一緒じゃね

    output.lightPosition =
    mul(worldPosition, gShadowTransform.lightWVP);

    return output;
}
