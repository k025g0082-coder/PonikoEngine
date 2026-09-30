struct VSInput
{
    float4 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};


struct ShadowTransform
{
    float4x4 world;
    float4x4 lightViewProjection;
};


ConstantBuffer<ShadowTransform> gShadow : register(b0);

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 worldPosition =
        mul(input.position, gShadow.world);

    output.position =
        mul(worldPosition, gShadow.lightViewProjection);

    return output;
}