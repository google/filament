// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s


// t3.b.x
// CHECK: [3 x float] [float 0.000000e+00, float 2.500000e+01, float 0.000000e+00]
// t3.b.y
// CHECK: [3 x float] [float 0.000000e+00, float 2.600000e+01, float 0.000000e+00]
// t3.c.x
// CHECK: constant [3 x i32] [i32 0, i32 27, i32 0]
// t3.c.y
// CHECK: [3 x i32] [i32 0, i32 28, i32 0]
// t3.a

// CHECK-DAG: [12 x float] [float 5.000000e+00, float 7.000000e+00, float 6.000000e+00, float 8.000000e+00, float 2.500000e+01, float 2.700000e+01, float 2.600000e+01, float 2.800000e+01, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00]

// t3.t

// CHECK-DAG: [24 x float] [float 2.500000e+01, float 2.700000e+01, float 2.600000e+01, float 2.800000e+01, float 2.500000e+01, float 2.700000e+01, float 2.600000e+01, float 2.800000e+01, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00, float 5.000000e+00, float 7.000000e+00, float 6.000000e+00, float 8.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 5.000000e+00, float 7.000000e+00, float 6.000000e+00, float 8.000000e+00]



static float4 f0 = {5,6,7,8};
static float4 f1 = 3;
static float4 f2 = {0,0,0,0};
static float4 f3[] = { f0, f1, f2 };

static float2x2 m2[4] = { 25,26,27,28,
                       25,26,27,28,
                       25,26,27,28,
                       25,26,27,28 };

struct T {
   float2x2 a;
};

struct T2 : T {
   float2 b;
   int2   c;
};

struct T3 : T2 {
   T t[2];
};

static T3 t3[] = { { f0, f2, m2 }, { f1, f3, f2, f0} };

uint i;

float4 main() : SV_TARGET {
  return t3[i].a[i][i] + t3[i].b.xxyy + t3[i].c.xyxy + t3[i].t[i].a[i][i];
}
