struct VSOut
{
	float4 pos : SV_Position;
	float4 WorldPosition : POSITION0;
	float3 color : Color;
	float2 ScreenPos : SC;
};

cbuffer VSContantBufferLayout : register(b0)
{
	float4x4 TransformMatrix;
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
	float4x4 PrevView;
	float4x4 PrevProjection;
};

VSOut main( float3 inpos : POSITION , float3 incolor : Color )
{
	VSOut Out;

	Out.pos = float4(inpos, 1.0f);
	Out.color = incolor;

	Out.pos = mul(TransformMatrix, Out.pos);
	Out.WorldPosition = Out.pos;

	Out.pos = mul(ViewMatrix, Out.pos);
	Out.pos = mul(ProjectionMatrix, Out.pos);

	Out.ScreenPos = (Out.pos.xy / Out.pos.w + 1) / 2;

	return Out;
}