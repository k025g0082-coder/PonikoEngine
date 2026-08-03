
#include "resources/Shader/Model.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
    
    int isSkinning;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register( b0 );

// なんでかっこ？ｗ
cbuffer Skinning : register(b2)
{
    float4x4 gBoneMatrices[128];
};

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
    
    //uint4 boneIndex : BLENDINDICES;　　　// PSにはこの情報はいらないので消す！！！！！！！！！！！！！
  //  float4 weight : BLENDWEIGHT;
    
};



VeltexShaderOutput main( VertexShaderInput input )
{
    VeltexShaderOutput output;
    
    // アニメありなら
    if (gTransformationMatrix.isSkinning != 0)
    {
        // ボーンの情報をもとに計算？
    
        float4 skinnedPosition = float4(0, 0, 0, 0);
        float3 skinnedNormal = float3(0, 0, 0);
        
        for (int i = 0; i < 4; i++)
        {
            skinnedPosition +=
            mul(input.position,
                gBoneMatrices[input.boneIndex[i]]
            )
            * input.weight[i];
            
            skinnedNormal +=
            mul(input.normal, (float3x3) gBoneMatrices[input.boneIndex[i]])
            * input.weight[i];
        }
        output.position = mul( skinnedPosition,gTransformationMatrix.WVP);
        output.texcoord = input.texcoord;
        output.normal =
        normalize(
            mul(skinnedNormal, (float3x3) gTransformationMatrix.World)
        );
    }
    else
    {
        output.position = mul(input.position, gTransformationMatrix.WVP);
        output.texcoord = input.texcoord;
        output.normal =
        normalize(
            mul(input.normal, (float3x3) gTransformationMatrix.World)
        );
    }
    
    return output;
}
