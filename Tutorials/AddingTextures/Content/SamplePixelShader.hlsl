// �ȼ� ���̴��� ����� �ȼ��� �� �������Դϴ�.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

// (������) �� �����Ϳ� ���� ��� �Լ��Դϴ�.
float4 main(PixelShaderInput input) : SV_TARGET
{
	return float4(input.color, 1.0f);
	//return g_texture.Sample(g_sampler, input.uv);
}
