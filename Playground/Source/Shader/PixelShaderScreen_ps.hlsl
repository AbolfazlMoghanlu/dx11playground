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

	float Padding[36];
};

Texture2D<float4> ReconTexture : register(t4);
SamplerState TextureSampler: register(s0);
SamplerState NearestSampler: register(s1);

float4 main(float4 pos : SV_Position, float4 WorldPosition : POSITION0, float3 color : Color,
	float2 ScreenPos : SC) : SV_TARGET
{
	float2 UVs = float2(ScreenPos.x, 1 - ScreenPos.y);
	float4 Color = ReconTexture.Sample(TextureSampler, UVs);
	return Color;
}