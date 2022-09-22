Texture2D<float4> CoverageTexture : register(t0);
SamplerState CoverageTextureSampler: register(s0);

float4 main(float4 pos : SV_Position, float4 WorldPosition : POSITION0, float3 color : Color) : SV_TARGET
{
	float2 UVs = WorldPosition.zx / 50000.0f;

	float4 CoverageSampled = CoverageTexture.Sample(CoverageTextureSampler, UVs);

	return float4(CoverageSampled.ggg, 1.0f);
}