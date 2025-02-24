// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: cbufferLoadLegacy
// CHECK: i32 0
// CHECK: extractvalue
// CHECK: , 0
// CHECK: extractvalue
// CHECK: , 1
// CHECK: cbufferLoadLegacy
// CHECK: i32 0
// CHECK: extractvalue
// CHECK: , 0
// CHECK: extractvalue
// CHECK: , 1
// CHECK: cbufferLoadLegacy
// CHECK: i32 8
// CHECK: extractvalue
// CHECK: , 0
// CHECK: cbufferLoadLegacy
// CHECK: i32 15
// CHECK: extractvalue
// CHECK: , 0
// CHECK: cbufferLoadLegacy
// CHECK: i32 22
// CHECK: extractvalue
// CHECK: , 0
// CHECK: cbufferLoadLegacy
// CHECK: i32 9
// CHECK: extractvalue
// CHECK: , 0
// CHECK: cbufferLoadLegacy
// CHECK: i32 14
// CHECK: extractvalue
// CHECK: , 0


struct ST {
   float4 a;
   float4 b;
};

struct cc {
  float2 cc1;
  float  cc2;
};

cbuffer T {
   float2  t;
   cc      cc0;
   row_major float1x2 f12;
   // Will change to ca[24].
   float  ca[2][3][4];
};

cbuffer T2 {
   float2 t2;
   float2x1 f21;
};

struct M1 {
float1x2 f12;
};

struct M2 {
row_major float1x2 f12;
};

float1x2 m1[2];
row_major float1x2 m2[2];

cc  c ;
float m : register(c8);

float4 main() : SV_TARGET
{
  float4 t3 = float4(t, t2);
  return t3 + m + c.cc1.x + ca[1][1][3] + m1[0][0][0] + m2[1][0][0];
}
