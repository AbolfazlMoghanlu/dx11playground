float4 main(float4 pos : SV_Position, float4 WorldPosition : POSITION0, float3 color : Color) : SV_TARGET
{
	return float4(color, 1.0f);
}