// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: main

struct T {
   row_major float3x4 m : A;
   float3x4 m2 : B;
   float4 pos: SV_Position;
};


T main(float4 a : A)
{
  T t;
  t.pos = a;
  t.m[0] = a;
  t.m[1] = a;
  t.m[2] = a;
  t.m2[0] = a;
  t.m2[1] = a;
  t.m2[2] = a;
  return t;
}

