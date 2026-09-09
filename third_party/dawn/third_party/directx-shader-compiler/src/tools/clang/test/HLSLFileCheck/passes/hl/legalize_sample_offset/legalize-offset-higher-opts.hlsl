// RUN: %dxc /Tps_6_0 /Emain  %s | FileCheck %s

// Make sure that the sample offsets get legalized even when loop is not unrolled
// for higher optimizations.
// CHECK: define void @main()
// CHECK: entry

sampler g_pointSampler : register(s0);
Texture2D s0 : register(t0);

float4 main( float2 input : A ) : SV_TARGET
{   
	float4 result = float4(1.0, 1.0, 1.0, 1.0);
	for( int y = -1; y <= 1; y++ ) 
	{
		for( int x = -1; x <= 1; x++ )
		{
			result += s0.Sample( g_pointSampler, input, int2(x,y) );
			
		}
	}

	return result;
}