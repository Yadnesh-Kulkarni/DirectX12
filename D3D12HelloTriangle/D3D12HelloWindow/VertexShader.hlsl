struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain( float4 pos : POSITION , float4 color : COLOR) 
{
	PSInput ToPS;

	ToPS.position = pos;
	ToPS.color = color;

	return ToPS;
}