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
	float BottomDensity;

	float TopDensity;
	float BaseNoiseScale;
	float DetailNoiseScale;
	float Anvil;

	float TracingStartMaxDistance;
	float Useless1;
	float Useless2;
	float Useless3;
};

Texture2D<float4> CoverageTexture : register(t0);
Texture3D<float4> WorleyTexture : register(t1);
Texture3D<float4> WorleyDetailTexture : register(t2);
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

	// --------------------------------------------------------------

	float PlanetRadius = 6360000.0f;
	float3 Origin = float3(0, StartZ - PlanetRadius, 0);

	float3 CameraToOrigin = Origin - CameraPosition;

	float a = dot(CameraVector, CameraToOrigin);
	float3 b = CameraPosition + CameraVector * a;
	float c = length(Origin - b);

	float e1 = sqrt(PlanetRadius * PlanetRadius - c * c);
	float3 NearIntersection = b + CameraVector * e1;

	if(length(NearIntersection - CameraPosition) > TracingStartMaxDistance)
		return float4(0.0f, 0, 0, 0);

	float FarRadius = PlanetRadius + Height;

	float e2 = sqrt(FarRadius * FarRadius - c * c);
	float3 FarIntersection = b + CameraVector * e2;

	float LayerHeight = length(FarIntersection - NearIntersection);

	float4 SS;
	SS = CoverageTexture.Sample(CoverageTextureSampler, NearIntersection.zx / 100 / CoveragemapScale).xxxx;
	return SS;

	// --------------------------------------------------------------

	float3 StepVector = CameraVector * T / Steps;

	float3 CurrentWorldPosition = WorldPosition;

	float Density = 0.0f;

	for(int i = 0; i < Steps; i++)
	{
		float ZGradient = (CurrentWorldPosition.y - StartZ) / Height;

		float2 UVs = CurrentWorldPosition.zx / CoveragemapScale / 10.0;
		float4 CoverageSampled = CoverageTexture.Sample(CoverageTextureSampler, UVs);

		float WMC = max(CoverageSampled.r, saturate(Coverage - 0.5f) * CoverageSampled.g * 2);

		float BottomCoverage = saturate(Remap(ZGradient, 0.0f, BottomRoundness, 0.0f, 1.0f));
		float TopCoverage = saturate(Remap(ZGradient, CoverageSampled.b * TopRoundness, CoverageSampled.b, 1.0f, 0.0f));
		float SA = BottomCoverage * TopCoverage;
		float SAA = pow(SA, saturate(Remap(ZGradient, 0.65, 0.95, 1, 1 - Anvil * Coverage)));

		float DensityOverZBottom = ZGradient * saturate(Remap(ZGradient, 0.0f, BottomDensity, 0.0f, 1.0f));
		float DensityOverZTop = saturate(Remap(ZGradient, TopDensity, 1.0f, 1.0f, 0.0f));
		float DA = DensityOverZBottom * DensityOverZTop;
		float DAA = DA * lerp(1, saturate(Remap(sqrt(ZGradient), 0.4, 0.95, 1, 0.2)), Anvil);
		
		float4 Worley = WorleyTexture.Sample(CoverageTextureSampler, CurrentWorldPosition / BaseNoiseScale);
		float4 WorleyDetail = WorleyDetailTexture.Sample(CoverageTextureSampler, CurrentWorldPosition / DetailNoiseScale);

		float SNS = Remap(Worley.r, (Worley.g * 0.625 + Worley.b * 0.25 + Worley.a * 0.125) - 1, 1, 0, 1);
		float DN = WorleyDetail.r * 0.625 + WorleyDetail.g * 0.25 + WorleyDetail.b * 0.125;

		float SN = saturate(Remap(SNS * SAA, 1 - Coverage * WMC, 1, 0, 1));
		float DNM = 0.35 * exp(-Coverage * 0.75) * lerp(DN, 1 - DN, saturate(ZGradient * 5));
	

		float d = saturate(Remap(SN, DNM, 1, 0, 1)) * DAA;


		Density += d * DensityScale;

		CurrentWorldPosition += StepVector;
	}

	return (1.0f);

	return float4(CloudColor, Density);
}