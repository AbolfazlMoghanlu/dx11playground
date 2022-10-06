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

Texture2D<float4> DownSampledTexture : register(t3);
Texture2D<float4> ReconTexture1 : register(t4);
Texture2D<float4> ReconTexture2 : register(t5);
SamplerState TextureSampler: register(s0);
SamplerState NearestSampler: register(s1);

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
		if (SwapIndex == 0)
		{
			Color = ReconTexture1.Sample(NearestSampler, UVs);
		}

		else
		{
			Color = ReconTexture2.Sample(NearestSampler, UVs);
		}
	}

	return Color;
}