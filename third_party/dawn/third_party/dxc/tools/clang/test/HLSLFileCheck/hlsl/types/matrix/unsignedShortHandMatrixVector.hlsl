// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: uint2
// CHECK: uint4x4
// CHECK: uint3x4
// CHECK: min16ui3
// CHECK: uint64_t2

unsigned int2 a;
unsigned int4x4 b;
unsigned dword3x4 c;
unsigned min16int3 d;
unsigned int64_t2 e;

float4 main() : SV_Target 
{
  return a.x + b[1][0] + c[0][3] + d[2];
}
