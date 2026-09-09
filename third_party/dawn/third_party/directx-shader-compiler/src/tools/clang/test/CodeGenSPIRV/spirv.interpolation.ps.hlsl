// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Required by the sample interpolation mode
// CHECK: OpCapability SampleRateShading

struct PSInput {
                       float   fp_a: FPA;
  linear               float1  fp_b: FPB;
// CHECK: OpDecorate %in_var_FPC Centroid
  centroid             float2  fp_c: FPC;
// CHECK: OpDecorate %in_var_FPD Flat
  nointerpolation      float3  fp_d: FPD;
// CHECK: OpDecorate %in_var_FPE NoPerspective
  noperspective        float4  fp_e: FPE;
// CHECK: OpDecorate %in_var_FPF Sample
  sample               float   fp_f: FPF;
// CHECK: OpDecorate %in_var_FPG NoPerspective
// CHECK: OpDecorate %in_var_FPG Sample
  noperspective sample float2  fp_g: FPG;

// CHECK: OpDecorate %in_var_INTA Flat
                       int    int_a: INTA;
// CHECK: OpDecorate %in_var_INTD Flat
  nointerpolation      int3   int_d: INTD;

// CHECK: OpDecorate %in_var_UINTA Flat
                       uint  uint_a: UINTA;
// CHECK: OpDecorate %in_var_UINTD Flat
  nointerpolation      uint3 uint_d: UINTD;

// CHECK: OpDecorate %in_var_BOOLA Flat
                       bool  bool_a: BOOLA;
// CHECK: OpDecorate %in_var_BOOLD Flat
  nointerpolation      bool3 bool_d: BOOLD;

// CHECK: OpDecorate %in_var_FPH Flat
  nointerpolation      float4 fp_h[1]: FPH;
};

float4 main(                     PSInput input,
                                 float   fp_a: FPA1,
            linear               float1  fp_b: FPB1,
// CHECK: OpDecorate %in_var_FPC1 Centroid
            centroid             float2  fp_c: FPC1,
// CHECK: OpDecorate %in_var_FPD1 Flat
            nointerpolation      float3  fp_d: FPD1,
// CHECK: OpDecorate %in_var_FPE1 NoPerspective
            noperspective        float4  fp_e: FPE1,
// CHECK: OpDecorate %in_var_FPF1 Sample
            sample               float   fp_f: FPF1,
// CHECK: OpDecorate %in_var_FPG1 NoPerspective
// CHECK: OpDecorate %in_var_FPG1 Sample
            noperspective sample float2  fp_g: FPG1,

// CHECK: OpDecorate %in_var_INTA1 Flat
                                 int    int_a: INTA1,
// CHECK: OpDecorate %in_var_INTD1 Flat
            nointerpolation      int3   int_d: INTD1,

// CHECK: OpDecorate %in_var_UINTA1 Flat
                                 uint   uint_a: UINTA1,
// CHECK: OpDecorate %in_var_UINTD1 Flat
            nointerpolation      uint3  uint_d: UINTD1,

// CHECK: OpDecorate %in_var_BOOLA1 Flat
                                 bool   bool_a: BOOLA1,
// CHECK: OpDecorate %in_var_BOOLD1 Flat
            nointerpolation      bool3  bool_d: BOOLD1
           ) : SV_Target {
  return 1.0;
}
