cbuffer PSConstantBuffer : register(b0)
{
	float3 CameraPosition;
	float UselessData;
}

Texture2D<float4> CoverageTexture : register(t0);
SamplerState CoverageTextureSampler: register(s0);



float4 main(float4 pos : SV_Position, float4 WorldPosition : POSITION0, float3 color : Color) : SV_TARGET
{
	const float H = 5000.0f;
	const int Steps = 64;
	const float CoverageMapScale = 50000.0f;

	float3 CameraVector = normalize(WorldPosition - CameraPosition);
	float T = sqrt(CameraVector.x * CameraVector.x + CameraVector.z * CameraVector.z);
	T = sqrt(CameraVector.y * CameraVector.y + T * T);

	float3 StepVector = CameraVector * T * H / Steps;
	
	float3 CurrentWorldPosition = WorldPosition;

	float Density = 0.0f;

	for(int i = 0; i < Steps; i++)
	{
		float2 UVs = CurrentWorldPosition.zx / CoverageMapScale;
		float4 CoverageSampled = CoverageTexture.Sample(CoverageTextureSampler, UVs);

		Density += CoverageSampled.g;

		CurrentWorldPosition += StepVector;
	}

	Density /= Steps;

	return Density.xxxx;

	//return float4(CoverageSampled.ggg, 1.0f);
}