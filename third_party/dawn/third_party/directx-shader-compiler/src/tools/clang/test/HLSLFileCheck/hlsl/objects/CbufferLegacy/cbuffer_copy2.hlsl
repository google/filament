// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure no alloca to copy.
// CHECK-NOT: alloca

cbuffer T
{
	float4 a[1];
	float4 b[2];
}
static const struct
{
	float4 a[1];
	float4 b[2];
} ST = { a, b};

uint i;

float4 main() : SV_Target
{
  return ST.a[i] + ST.b[i];
}
