struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORDS;
};

PSInput VSMain( float4 pos : POSITION , float2 uv : TEXCOORD) 
{
	PSInput ToPS;

	ToPS.position = pos;
	ToPS.uv = uv;

	return ToPS;
}