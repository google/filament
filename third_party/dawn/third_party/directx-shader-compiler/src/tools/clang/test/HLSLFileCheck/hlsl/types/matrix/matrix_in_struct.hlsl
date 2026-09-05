// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: main

struct T {
   float4x4 m;

   row_major float4x4 m1;
   column_major float4x4 m2;
};

T g_t;

T getT() {
  return g_t;
}

float4 main(float4 a : A) : SV_TARGET
{
  T t = getT();
  int4x4 im = 2;
  im[2] = 1;
  return im[2] + t.m[1] + t.m2[1] + t.m1[1] + t.m2._m00 + t.m1._m01;
}

