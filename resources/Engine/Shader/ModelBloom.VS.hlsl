
#include "resources/Engine/Shader/ModelBloom.hlsli"


// メインのモデルVSとほぼ同じ


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

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

// なんでかっこ？ｗ
cbuffer Skinning : register(b1)
{
    float4x4 gBoneMatrices[128];
};

ConstantBuffer<ModelSkinningParam> gModelSkinningParam : register(b2); // b数字はﾊﾟｲﾌﾟﾗｲﾝの数字と一致させるように


struct VertexShaderInput
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
  
    uint4 boneIndex : BLENDINDICES;
    float4 weight : BLENDWEIGHT;
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
    
    
    return output;
}
