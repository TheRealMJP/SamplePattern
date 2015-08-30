//======================================================================
//
//	MSAA Sample Pattern Inspector
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#if MSAAEnabled
	Texture2DMS<float4, NumSamples> PatternTexture : register(t0);
#else
	Texture2D<float4> PatternTexture : register(t0);
#endif

cbuffer Constants : register(b2)
{
	uint2 PixelPosition;
}

float4 PS() : SV_Target
{    
	#if MSAAEnabled
		return PatternTexture.Load(PixelPosition, SampleIndex);    
	#else
		return PatternTexture.Load(int3(PixelPosition, 0));    
	#endif
}