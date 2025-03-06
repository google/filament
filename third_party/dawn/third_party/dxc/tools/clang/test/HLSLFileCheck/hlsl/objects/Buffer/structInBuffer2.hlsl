// RUN: %dxc -E main  -T cs_6_0 %s | FileCheck %s

// CHECK: @main
struct Foo {
    int a[2];
    int d[2];
};

Buffer<Foo> inputs : register(t1);
RWBuffer< int > g_Intensities : register(u1);

groupshared Foo sharedData;

[ numthreads( 64, 2, 2 ) ]
void main( uint GI : SV_GroupIndex)
{
  if (GI==0)
	sharedData = inputs[GI];
	int rtn;
	InterlockedAdd(sharedData.d[0], g_Intensities[GI], rtn);
	g_Intensities[GI] = rtn + sharedData.d[0];
}