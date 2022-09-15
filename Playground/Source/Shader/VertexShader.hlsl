struct VSOut
{
	float4 pos : SV_Position;
	float3 color : Color;
};

cbuffer VSContantBufferLayout : register(b0)
{
	matrix ViewMatrix;
};

VSOut main( float3 inpos : POSITION , float3 incolor : Color )
{
	VSOut Out;

	Out.pos = float4(inpos, 1.0f);
	Out.color = incolor;

	//Out.pos = mul(Out.pos, ViewMatrix);
	Out.pos = mul(ViewMatrix, Out.pos);

	Out.pos.z /= 1000.f;

	return Out;
}