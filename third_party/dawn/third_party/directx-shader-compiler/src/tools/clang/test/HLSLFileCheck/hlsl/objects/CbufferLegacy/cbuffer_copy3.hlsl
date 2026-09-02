// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure no alloca to copy.
// CHECK-NOT: alloca

struct S {
  float s;
};

cbuffer T
{
        S  s;
	float4 a[1];
	float4 b[2];
}
static const struct
{
        S  s;
	float4 a[1];
	float4 b[2];
} ST = { s, a, b};

uint i;

float4 main() : SV_Target
{
  return ST.a[i] + ST.b[i];
}
