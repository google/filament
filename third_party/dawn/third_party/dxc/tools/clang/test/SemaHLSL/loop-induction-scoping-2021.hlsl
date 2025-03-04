// RUN: %dxc -Tlib_6_3 -verify -HV 2021 %s
// RUN: %dxc -Tps_6_0 -verify -HV 2021 %s

// expected-no-diagnostics

uint g_count1;
uint g_count2;
Buffer<float4> Things;

float4 main() : SV_Target
{
	float4 data=0;
	for( uint i=0; i<g_count1; i++ )
	{
		data += Things[i];
	}

  // not-expected-warning@+1:{{redefinition of 'i'}}
	for( uint i=0; i<g_count2; i++ )
	{
		data += Things[g_count1+i];
	}

	return data;
}
