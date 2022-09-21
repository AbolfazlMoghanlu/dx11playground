struct VSOut
{
	float4 pos : SV_Position;
	float3 color : Color;
};

cbuffer VSContantBufferLayout : register(b0)
{
	float4x4 TransformMatrix;
	float4x4 ViewMatrix;
};

VSOut main( float3 inpos : POSITION , float3 incolor : Color )
{
	VSOut Out;

	Out.pos = float4(inpos, 1.0f);
	Out.color = incolor;

	//Out.pos = mul(Out.pos, TransformMatrix);
	Out.pos = mul(TransformMatrix, Out.pos);
	//Out.pos = mul(ViewMatrix, Out.pos);
	Out.pos = mul(Out.pos, ViewMatrix);

	//Out.pos.z = Out.pos.z / 5;

	return Out;
}