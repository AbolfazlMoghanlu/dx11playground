cbuffer PSConstantBuffer : register(b0)
{
	float3 CameraPosition;
	float UselessData;
}

cbuffer PCloudBufferLayout : register(b1)
{
	float3 CloudColor;
	float StartZ;

	int Steps;
	float Height;
	float Coverage;
	float CoveragemapScale;

	float DensityScale;
	float BottomRoundness;
	float TopRoundness;
	float BottomDensity = 0.15f;

	float TopDensity = 0.9f;
	float Useless1;
	float Useless2;
	float Useless3;
};

Texture2D<float4> CoverageTexture : register(t0);
SamplerState CoverageTextureSampler: register(s0);


float Remap(float V, float InMin, float InMax, float OutMin, float OutMax)
{
	return OutMin + (((V - InMin) * (OutMax - OutMin)) / (InMax - InMin));
}

float4 main(float4 pos : SV_Position, float4 WorldPosition : POSITION0, float3 color : Color) : SV_TARGET
{
	float3 CameraToWorldPosition = (WorldPosition - CameraPosition);
	float3 CameraVector = normalize(CameraToWorldPosition);

	float SkyCaemraRelativeZ = StartZ - CameraPosition.y;
	float PixelDistance = length(CameraToWorldPosition);
	
	float T = PixelDistance * (Height / SkyCaemraRelativeZ);

	float3 StepVector = CameraVector * T / Steps;

	float3 CurrentWorldPosition = WorldPosition;

	float Density = 0.0f;

	for(int i = 0; i < Steps; i++)
	{
		float ZGradient = (CurrentWorldPosition.y - StartZ) / Height;

		float2 UVs = CurrentWorldPosition.zx / CoveragemapScale / 10.0;
		float4 CoverageSampled = CoverageTexture.Sample(CoverageTextureSampler, UVs);

		float CoverageValue = max(CoverageSampled.r, saturate(Coverage - 0.5f) * CoverageSampled.g * 2);

		float BottomCoverage = saturate(Remap(ZGradient, 0.0f, BottomRoundness, 0.0f, 1.0f));
		float TopCoverage = saturate(Remap(ZGradient, CoverageSampled.b * TopRoundness, CoverageSampled.b, 1.0f, 0.0f));

		float DensityOverZBottom = ZGradient * saturate(Remap(ZGradient, 0.0f, BottomDensity, 0.0f, 1.0f));
		float DensityOverZTop = saturate(Remap(ZGradient, TopDensity, 1.0f, 1.0f, 0.0f));

		
		
		Density += CoverageValue * DensityScale * BottomCoverage * TopCoverage * DensityOverZBottom * DensityOverZTop * CoverageSampled.a;

		CurrentWorldPosition += StepVector;
	}

	return float4(CloudColor, Density);
}