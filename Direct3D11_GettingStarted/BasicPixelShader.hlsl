struct psInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(psInput ps) : SV_TARGET
{
    return ps.color;
}