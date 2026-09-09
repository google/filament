// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure no alloca to copy.
// CHECK-NOT: alloca

struct M {
  float4x4  m;
};

cbuffer T
{
	M a[2];
	float4 b[2];
}
struct ST
{
	M a[2];
	float4 b[2];
};

uint i;

float4 main() : SV_Target
{
  ST st;
  st.a = a;
  st.b = b;
  return mul(st.a[i].m, st.b[i]);
}
