Texture2D shaderTexture;
SamplerState SampleType;

struct psInput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(psInput ps) : SV_TARGET
{
    float4 texColor;
    texColor = shaderTexture.Sample(SampleType, ps.tex);
    return texColor;
}