// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure addrspacecast is cleaned up.
// CHECK: @main()
// CHECK-NOT: addrspacecast
// CHECK: ret void

struct ST
{
	float3 a; // center
	float3 b; // half extents

        void func(float3 x, float3 y)
	{
		a = x + y;
		b = x * y;
	}
};

groupshared ST myST;
StructuredBuffer<ST> buf0;
float3 a;
float3 b;
RWBuffer<float3> buf1;
[numthreads(8,8,1)]
void main() {
  myST = buf0[0];
  myST.func(a, b);
  buf1[0] = myST.b;
}