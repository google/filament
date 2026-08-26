// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: cbufferLoadLegacy
// CHECK: 0)
// CHECK: extractvalue
// CHECK: 1
// CHECK: cbufferLoadLegacy
// CHECK: 1)
// CHECK: extractvalue
// CHECK: 1
// CHECK: cbufferLoadLegacy
// CHECK: 2)
// CHECK: extractvalue
// CHECK: 1
// CHECK: cbufferLoadLegacy
// CHECK: 3)
// CHECK: extractvalue
// CHECK: 1


// CHECK: cbufferLoadLegacy
// CHECK: 8)
// CHECK: extractvalue
// CHECK: 1
// CHECK: cbufferLoadLegacy
// CHECK: 9)
// CHECK: extractvalue
// CHECK: 1
// CHECK: cbufferLoadLegacy
// CHECK: 10)
// CHECK: extractvalue
// CHECK: 1
// CHECK: cbufferLoadLegacy
// CHECK: 11)
// CHECK: extractvalue
// CHECK: 1

// CHECK: cbufferLoadLegacy
// CHECK: 5)
// CHECK: extractvalue
// CHECK: 0
// CHECK: extractvalue
// CHECK: 1
// CHECK: extractvalue
// CHECK: 2
// CHECK: extractvalue
// CHECK: 3

struct T {
   float4x4 m;

   row_major float4x4 m1;
   column_major float4x4 m2;
};

T t;

float4 main(float4 a : A) : SV_TARGET
{
  int4x4 im = 2;
  im[2] = 1;
  return im[2] + t.m[1] + t.m2[1] + t.m1[1] + t.m2._m00 + t.m1._m01;
}

