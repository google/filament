// RUN: %dxc -T vs_6_0 -E main -pack-optimized -O0  %s -spirv | FileCheck %s

struct VS_OUTPUT {
  float4 pos : SV_POSITION;

// CHECK-DAG: OpDecorate %out_var_A Location 0
  float a : A;

// CHECK-DAG: OpDecorate %out_var_B Location 0
// CHECK-DAG: OpDecorate %out_var_B Component 2
  double b : B;

// CHECK-DAG: OpDecorate %out_var_C Location 1
// CHECK-DAG: OpDecorate %out_var_C Component 0
// CHECK-DAG: OpDecorate %out_var_C_0 Location 2
// CHECK-DAG: OpDecorate %out_var_C_0 Component 0
// CHECK-DAG: OpDecorate %out_var_C_1 Location 3
// CHECK-DAG: OpDecorate %out_var_C_1 Component 0
  float2 c[3] : C;

// CHECK-DAG: OpDecorate %out_var_D Location 1
// CHECK-DAG: OpDecorate %out_var_D Component 2
// CHECK-DAG: OpDecorate %out_var_D_0 Location 2
// CHECK-DAG: OpDecorate %out_var_D_0 Component 2
  float2x2 d : D;

// CHECK-DAG: OpDecorate %out_var_E Location 3
// CHECK-DAG: OpDecorate %out_var_E Component 2
  int e : E;

// CHECK-DAG: OpDecorate %out_var_F Location 4
  float2 f : F;

// CHECK-DAG: OpDecorate %out_var_G Location 3
// CHECK-DAG: OpDecorate %out_var_G Component 3
  float g : G;
};

VS_OUTPUT main(float4 pos : POSITION,
               float4 color : COLOR) {
  VS_OUTPUT vout;
  return vout;
}
