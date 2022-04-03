struct vsInput
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct psInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

psInput main(vsInput psi)
{
    psInput pso;
    pso.color = psi.color;
    pso.position = psi.position;
	return pso;
}