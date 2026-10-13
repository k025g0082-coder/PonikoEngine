struct VSInput
{
    float4 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};


struct TransformationMatrix
{
    float4x4 World;
    float4x4 WVP;
};



ConstantBuffer<TransformationMatrix> gTransform : register(b0);

VSOutput main(VSInput input)
{
    VSOutput output;

    output.position = mul(input.position, gTransform.WVP);

    return output;
}