// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Required by the sample interpolation mode
// CHECK: OpCapability SampleRateShading

struct VSOutput {
                       float   fp_a: FPA;
  linear               float1  fp_b: FPB;
// CHECK: OpDecorate %out_var_FPC Centroid
  centroid             float2  fp_c: FPC;
// CHECK: OpDecorate %out_var_FPD Flat
  nointerpolation      float3  fp_d: FPD;
// CHECK: OpDecorate %out_var_FPE NoPerspective
  noperspective        float4  fp_e: FPE;
// CHECK: OpDecorate %out_var_FPF Sample
  sample               float   fp_f: FPF;
// CHECK: OpDecorate %out_var_FPG NoPerspective
// CHECK: OpDecorate %out_var_FPG Sample
  noperspective sample float2  fp_g: FPG;

// CHECK: OpDecorate %out_var_INTA Flat
                       int    int_a: INTA;
// CHECK: OpDecorate %out_var_INTD Flat
  nointerpolation      int3   int_d: INTD;

// CHECK: OpDecorate %out_var_UINTA Flat
                       uint  uint_a: UINTA;
// CHECK: OpDecorate %out_var_UINTD Flat
  nointerpolation      uint3 uint_d: UINTD;

// CHECK: OpDecorate %out_var_BOOLA Flat
                       bool  bool_a: BOOLA;
// CHECK: OpDecorate %out_var_BOOLD Flat
  nointerpolation      bool3 bool_d: BOOLD;

// CHECK: OpDecorate %out_var_FPH Flat
  nointerpolation      float4 fp_h[1]: FPH;
};

// CHECK: OpDecorate %out_var_TEXCOORD NoPerspective
VSOutput main(out noperspective int a : TEXCOORD) {
  VSOutput myOutput;
  return myOutput;
}

