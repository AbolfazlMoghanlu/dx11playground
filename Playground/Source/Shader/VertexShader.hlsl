struct VSOut
{
	float4 pos : SV_Position;
	float3 color : Color;
};

VSOut main( float3 inpos : POSITION , float3 incolor : Color )
{
	VSOut Out;

	Out.pos = float4(inpos, 1.0f);
	Out.color = incolor;

	return Out;
}