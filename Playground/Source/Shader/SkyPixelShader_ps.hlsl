cbuffer PCloudBufferLayout : register(b1)
{
	float3 CameraPosition;
	float UselessData;

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
	float DetailNoiseIntensity;
	int LightSteps;
	float LightStepSize;

	float3 LightDir;
	float Useless3;

	float Padding[36];
};

Texture2D<float4> CoverageTexture : register(t0);
Texture3D<float4> WorleyTexture : register(t1);
Texture3D<float4> WorleyDetailTexture : register(t2);
SamplerState CoverageTextureSampler: register(s0);

float Remap(float V, float InMin, float InMax, float OutMin, float OutMax)
{
	return OutMin + (((V - InMin) * (OutMax - OutMin)) / (InMax - InMin));
}

float DensityAtPosition(float3 Position, float LayerHeight)
{
	float ZGradient = LayerHeight;

	float2 UVs = Position.zx / CoveragemapScale;
	float4 CoverageSampled = CoverageTexture.Sample(CoverageTextureSampler, UVs);

	float WMC = max(CoverageSampled.r, saturate(Coverage - 0.5f) * CoverageSampled.g * 2);

	float BottomCoverage = saturate(Remap(ZGradient, 0.0f, BottomRoundness, 0.0f, 1.0f));
	float TopCoverage = saturate(Remap(ZGradient, CoverageSampled.b * TopRoundness, CoverageSampled.b, 1.0f, 0.0f));
	float SA = BottomCoverage * TopCoverage;

	float DensityOverZBottom = ZGradient * saturate(Remap(ZGradient, 0.0f, BottomDensity, 0.0f, 1.0f));
	float DensityOverZTop = saturate(Remap(ZGradient, TopDensity, 1.0f, 1.0f, 0.0f));
	float DA = DensityOverZBottom * DensityOverZTop;

	float4 Worley = WorleyTexture.Sample(CoverageTextureSampler, Position.xzy / BaseNoiseScale);
	float4 WorleyDetail = WorleyDetailTexture.Sample(CoverageTextureSampler, Position.xzy / DetailNoiseScale);

	//return CoverageSampled.x * DensityScale;

	float SNS = Remap(Worley.r, (Worley.g * 0.625 + Worley.b * 0.25 + Worley.a * 0.125) - 1, 1, 0, 1);
	float DN = WorleyDetail.r * 0.625 + WorleyDetail.g * 0.25 + WorleyDetail.b * 0.125;

	float SN = saturate(Remap(SNS * SA, 1 - Coverage * WMC, 1, 0, 1));
	float DNM = DetailNoiseIntensity * exp(-Coverage * 0.75) * lerp(DN, 1 - DN, saturate(ZGradient * 5));


	float d = saturate(Remap(SN, DNM, 1, 0, 1)) * DA;
	d *= DensityScale;

	return d;
}


float RayIntersection(float3 RayOrigin, float3 RayDir, float3 SphereOrigin, float SphereRadius)
{
	float3 RayOriginToSphereOrigin = SphereOrigin - RayOrigin;
	float a = dot(RayDir, RayOriginToSphereOrigin);
	float3 b = RayOrigin + RayDir * a;
	float c = length(SphereOrigin - b);

	float d = sqrt(SphereRadius * SphereRadius - c * c);
	return d + a;
}


float4 main(float4 pos : SV_Position, float4 WorldPosition : POSITION0, float3 color : Color) : SV_Target
{
float3 CameraToWorldPosition = WorldPosition.xyz - CameraPosition;
	float3 CameraVector = normalize(CameraToWorldPosition);

	// --------------------------------------------------------------

	float PlanetRadius = 6360000.0f;
	float3 Origin = float3(0, StartZ - PlanetRadius, 0);

	float t3 = RayIntersection(CameraPosition, CameraVector, Origin, PlanetRadius);

	float FarH = PlanetRadius + Height;
	float t4 = RayIntersection(CameraPosition, CameraVector, Origin, FarH);

	float3 NearIntersection = CameraPosition + CameraVector * t3;
	float3 FarIntersection = CameraPosition + CameraVector * t3;
	float LayerHeight = t4 - t3;

	if (length(NearIntersection - CameraPosition) > TracingStartMaxDistance)
		return float4(0.0, 0, 0, 0);

	float3 StepVector = CameraVector * LayerHeight / Steps;
	float3 CurrentWorldPosition = NearIntersection;
	float Density = 0.0f;

	bool FullOpace = false;

	float Trans = 0.0f;
	bool FullTrans = false;


	[loop] for (int i = 0; i < Steps; i++)
	{
		if (!FullOpace)
		{
			float LayerHeight = i > 0 ? (float)i / (Steps - 1) : 0.0f;

			float d = DensityAtPosition(CurrentWorldPosition, LayerHeight);
			Density = saturate(Density + d);

			FullOpace = Density > 0.99;
		}

		//if (!FullTrans)
		//{
		//	float3 CurrentLightPosition = CurrentWorldPosition;
		//	float3 LightStepVector = LightDir * LightStepSize / LightSteps / 100; 
		//
		//	[loop] for(int j = 0; j < LightSteps; j++)
		//	{
		//		if (!FullTrans)
		//		{
		//			float3 RayDirection = normalize(CurrentLightPosition - CameraPosition);
		//			float i1 = RayIntersection(CameraPosition, RayDirection, Origin, PlanetRadius);
		//			float i2 = RayIntersection(CameraPosition, RayDirection, Origin, FarH);
		//			float i3 = i2 - i1;
		//
		//			float LayerHeight = (length(CurrentLightPosition - CameraPosition) - i1) / i3;
		//
		//			float t = DensityAtPosition(CurrentLightPosition, LayerHeight);
		//			Trans = saturate(Trans + t);
		//
		//
		//			CurrentLightPosition = CurrentLightPosition * LightStepVector;
		//			FullTrans = Trans > 0.99;
		//		}
		//	}
		//}

		CurrentWorldPosition += StepVector;
	}

	//float3 Color = CloudColor * (1 - Trans);
	float3 Color = CloudColor;

	return float4(Color, Density);
}