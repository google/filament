// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s



// Make sure both channel is used.

// CHECK: extractvalue %dx.types.CBufRet.i32 {{.*}}, 0
// CHECK: extractvalue %dx.types.CBufRet.i32 {{.*}}, 1

RWBuffer<uint2> output;
uint2 a;
[numthreads(4, 4, 4)]
void main(uint3 iv : SV_GroupThreadID)
{
	output[0] = iv.x == 0 ? 0 : a;
}