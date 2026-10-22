// RUN: %dxc -E main -fcgl -T ps_6_0 /Gec -HV 2016 %s | FileCheck %s

// Writing to globals only supported with HV <= 2016

// CHECK: {{.*g_s1.*}} = external constant float, align 4
// CHECK: {{.*g_s2.*}} = external constant float, align 4
// CHECK: {{.*g_v.*}} = external constant <4 x float>, align 4
// CHECK: {{.*g_m1.*}} = external constant %class.matrix.int.2.2, align 4
// CHECK: {{.*g_m2.*}} = external constant %class.matrix.int.2.2, align 4
// CHECK: {{.*g_b.*}} = external constant i32, align 4
// CHECK: {{.*g_a.*}} = external constant [5 x i32], align 4
// CHECK: {{.*g_a2d.*}} = external constant [3 x [2 x i32]], align 4
// CHECK-NOT: {{(.*g_s1.*)(.*static.copy.*)}} = internal global float 0.000000e+00
// CHECK: {{(.*g_s2.*)(.*static.copy.*)}} = internal global float 0.000000e+00
// CHECK-NOT: {{(.*g_v.*)(.*static.copy.*)}} = internal global <4 x float> zeroinitializer
// CHECK-NOT: {{(.*g_m1.*)(.*static.copy.*)}} = internal global %class.matrix.int.2.2 zeroinitializer
// CHECK: {{(.*g_m2.*)(.*static.copy.*)}} = internal global %class.matrix.int.2.2 zeroinitializer
// CHECK-NOT: {{(.*g_b.*)(.*static.copy.*)}} = internal global i32 0
// CHECK-NOT: {{(.*g_a.*)(.*static.copy.*)}} = internal global [5 x i32] zeroinitializer
// CHECK-NOT: {{(.*g_a2d.*)(.*static.copy.*)}} = internal global [3 x [2 x i32]] zeroinitializer
// CHECK: define <4 x float> @main


float g_s1;
float g_s2; // write enabled
float4 g_v;
int2x2 g_m1;
int2x2 g_m2; // write enabled
bool g_b;
int g_a[5];
int g_a2d[3][2];
float4 main(uint a
            : A) : SV_Target {
  g_s2 = a * 2.0f;
  
  for (uint i = 0; i < 2; i++)
    for (uint j = 0; j < 2; j++)
      g_m2[i][j] = a + i + j;

      
  return float4(g_s1, g_s1, g_s2, g_s2) +
         g_v +
         float4(g_m1[0][0], g_m1[0][1], g_m2[1][0], g_m2[1][1]) +
         float4(g_a2d[0][0], g_a2d[0][1], g_a2d[1][0], g_a2d[1][1]) +
         float4(g_a[0], g_a[1], g_a[2], g_a[3]);
}