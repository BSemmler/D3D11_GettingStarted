struct vsInput
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct psInput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

psInput main(vsInput psi)
{
    psInput pso;
    pso.tex = psi.tex;
    pso.position = psi.position;
	return pso;
}