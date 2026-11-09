struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PostEffectParam
{
    float blurPower;
    float time;

    float hzWavePower;
    float vtWavePower;

    float monotone;
    float colorInversion;

    float noise;
    float neon;

    float scanLine;
    float fishLens;
    float glitch;

    float4 color;
};


struct BloomParam
{
    float bloomThreshold;
    float bloomIntensity;
    float bloomSoftness;
    float blurTexelSize; 
};

struct FogParam
{
    bool isFog;
    float3 fogColor;
    float fogStart;
    float fogEnd;
    float2 padding;
};