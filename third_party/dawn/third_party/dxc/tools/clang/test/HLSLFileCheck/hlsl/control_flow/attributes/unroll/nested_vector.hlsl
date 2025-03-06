// RUN: %dxc /Od /T lib_6_3 /exports UnrollTest %s | FileCheck %s
// RUN: %dxc /Od /T lib_6_3 /exports UnrollTest %s /Zi | FileCheck %s

// Check that we can do unroll properly using vectors

// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3

// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3

// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3

// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3
// CHECK: %{{[0-9]+}} = call float @dx.op.dot3

// CHECK-NOT: %{{[0-9]+}} = call float @dx.op.dot3

struct MyInt2 { int x, y; };

float UnrollTest(float3 a : A, float3 b : B)
{
	int4 offset;
  float ret = 0;
	[unroll] for (offset.x = 0; offset.x <= 1; ++offset.x)
	[unroll] for (offset.y = 0; offset.y <= 1; ++offset.y)
	[unroll] for (offset.z = 0; offset.z <= 1; ++offset.z)
	[unroll] for (offset.w = 0; offset.w <= 1; ++offset.w)
	{
    ret += dot(a, b);
  }
  return ret;
}


