// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: elements of typed buffers and textures must fit in four 32-bit quantities

struct Foo {
  float2 a;
  float b;
  float c;
  float d[2];
};

Buffer<Foo> inputs : register(t1);

RWBuffer< int > g_Intensities : register(u1);

[ numthreads( 64, 2, 2 ) ]
void main( uint GI : SV_GroupIndex)
{
	g_Intensities = inputs[GI].d[0];
}