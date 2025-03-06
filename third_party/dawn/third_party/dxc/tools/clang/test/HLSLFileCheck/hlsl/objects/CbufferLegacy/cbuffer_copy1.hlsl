// RUN: %dxc -E not_main -T ps_6_0 %s | FileCheck %s

// Make sure no load and store
// CHECK-NOT: store float
// CHECK-NOT: = load



cbuffer T
{
	float4 a;
	float4 b;
}
static struct X
{
	float4 a;
	float4 b;
} ST = { a, b};

uint t;

// Not use main as entry name to disable GlobalOpt.
float4 not_main() : SV_Target
{
  float tmp = 0;
  // Make big number of instructions to disable gvn.
  [unroll]
  for (uint i=0;i<100;i++)
    tmp += sin(t+i);

  return tmp + i + sin(ST.a) + ST.b;
}
