Texture2D<float4> CloudTexture : register(t0);
SamplerState TextureSampler: register(s0);

float4 main(float4 pos : SV_Position, float4 WorldPosition : POSITION0, float3 color : Color,
	float2 ScreenPos : SC) : SV_TARGET
{
	float4 Color = CloudTexture.Sample(TextureSampler, ScreenPos);

	return Color;
}