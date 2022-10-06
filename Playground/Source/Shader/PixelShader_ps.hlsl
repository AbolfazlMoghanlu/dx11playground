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
	int CBR;

	int SwapIndex;

	float Padding[35];
};

cbuffer VSContantBufferLayout : register(b0)
{
	float4x4 TransformMatrix;
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
	float4x4 PrevView;
	float4x4 PrevProjection;
};

Texture2D<float4> DownSampledTexture : register(t3);
Texture2D<float4> ReconTexture1 : register(t4);
Texture2D<float4> ReconTexture2 : register(t5);
SamplerState TextureSampler: register(s0);
SamplerState NearestSampler: register(s1);



float RayIntersection(float3 RayOrigin, float3 RayDir, float3 SphereOrigin, float SphereRadius)
{
	float3 RayOriginToSphereOrigin = SphereOrigin - RayOrigin;
	float a = dot(RayDir, RayOriginToSphereOrigin);
	float3 b = RayOrigin + RayDir * a;
	float c = length(SphereOrigin - b);

	float d = sqrt(SphereRadius * SphereRadius - c * c);
	return d + a;
}



float4 main(float4 pos : SV_Position, float4 WorldPosition : POSITION0, float3 color : Color,
	float2 ScreenPos : SC) : SV_TARGET
{
	float2 T = ScreenPos / float2(1.0/1080.0, 1.0/720.0); 
	int T1 = fmod((int)T.x, 4);
	int T2 = fmod((int)T.y, 4);

	int TX = fmod(CBR, 4);
	int TY = CBR / 4;


	float2 UVs = float2(ScreenPos.x, 1 - ScreenPos.y);
	float4 Color;

	if(TX == T1 && TY == T2)
	{
		Color = DownSampledTexture.Sample(NearestSampler, UVs);
	}

	else
	{
		float2 ReprojectedUV;
		float3 CameraVector = normalize(WorldPosition - CameraPosition);


		float PlanetRadius = 6360000.0f;
		float3 Origin = float3(0, StartZ - PlanetRadius, 0);

		float t3 = RayIntersection(CameraPosition, CameraVector, Origin, PlanetRadius);
		float3 NearIntersection = CameraPosition + CameraVector * t3 * 1;



		float4 P = mul(PrevView, float4(NearIntersection, 1));
		P = mul(PrevProjection, P);

		ReprojectedUV = P.xy / P.w;

		ReprojectedUV += 1;
		ReprojectedUV /= 2;
		ReprojectedUV = float2(ReprojectedUV.x, 1 - ReprojectedUV.y);

		//return float4(ReprojectedUV, 0, 1);

		if(ReprojectedUV.x < 0 || ReprojectedUV.x > 1 || ReprojectedUV.y < 0 || ReprojectedUV.y > 1)
			return float4(0, 0, 0, 0);

		if (SwapIndex == 0)
		{
			Color = ReconTexture1.Sample(NearestSampler, ReprojectedUV);
		}

		else
		{
			Color = ReconTexture2.Sample(NearestSampler, ReprojectedUV);
		}
	}






	return Color;
}