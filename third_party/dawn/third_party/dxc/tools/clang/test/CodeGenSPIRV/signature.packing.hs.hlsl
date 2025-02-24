// RUN: %dxc -T hs_6_0 -E main -pack-optimized -O0  %s -spirv | FileCheck %s

struct HSPatchConstData {
  float tessFactor[3] : SV_TessFactor;
  float insideTessFactor[1] : SV_InsideTessFactor;

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

struct HSCtrlPt {
// CHECK-DAG: OpDecorate %out_var_H Location 4
// CHECK-DAG: OpDecorate %out_var_H Component 2
  float h : H;

// CHECK-DAG: OpDecorate %out_var_I Location 5
  float2 i : I;

// CHECK-DAG: OpDecorate %out_var_J Location 6
// CHECK-DAG: OpDecorate %out_var_J Component 0
// CHECK-DAG: OpDecorate %out_var_J_0 Location 7
// CHECK-DAG: OpDecorate %out_var_J_0 Component 0
// CHECK-DAG: OpDecorate %out_var_J_1 Location 8
// CHECK-DAG: OpDecorate %out_var_J_1 Component 0
// CHECK-DAG: OpDecorate %out_var_J_2 Location 9
// CHECK-DAG: OpDecorate %out_var_J_2 Component 0
// CHECK-DAG: OpDecorate %out_var_J_3 Location 10
// CHECK-DAG: OpDecorate %out_var_J_3 Component 0
  float j[5] : J;

// CHECK-DAG: OpDecorate %out_var_K Location 6
// CHECK-DAG: OpDecorate %out_var_K Component 1
// CHECK-DAG: OpDecorate %out_var_K_0 Location 7
// CHECK-DAG: OpDecorate %out_var_K_0 Component 1
// CHECK-DAG: OpDecorate %out_var_K_1 Location 8
// CHECK-DAG: OpDecorate %out_var_K_1 Component 1
// CHECK-DAG: OpDecorate %out_var_K_2 Location 9
// CHECK-DAG: OpDecorate %out_var_K_2 Component 1
  float4x3 k : K;
};

HSPatchConstData HSPatchConstantFunc(const OutputPatch<HSCtrlPt, 3> input) {
  HSPatchConstData data;
  data.tessFactor[0] = 3.0;
  data.tessFactor[1] = 3.0;
  data.tessFactor[2] = 3.0;
  data.insideTessFactor[0] = 3.0;
  return data;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatchConstantFunc")]
[maxtessfactor(15)]
HSCtrlPt main(InputPatch<HSCtrlPt, 3> input, uint CtrlPtID : SV_OutputControlPointID) {
  HSCtrlPt data;
  data.k = input[CtrlPtID].k;
  return data;
}
