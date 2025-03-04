// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

float func(float e, float f, float g, float h);
float func2(float e, float f, float g, float h);
precise float func3(float e, float f, float g, float h);
float func4(float i, float j, precise out float k);

// The purpose of this to make sure the first NoContraction decoration is on a_mul_b.
// CHECK:      OpName %bb_entry_3
// CHECK-NEXT: OpDecorate [[a_mul_b:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[c_mul_d:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[r_plus_s:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[aw_mul_bw:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[cw_mul_dw:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[awbw_plus_cwdw:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func_ax_mul_bx:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func_cx_mul_dx:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func2_e_mul_f:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func2_g_mul_h:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func2_ef_plus_gh:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func3_e_mul_f:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func3_g_mul_h:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func3_ef_plus_gh:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func4_i_mul_i:%[0-9]+]] NoContraction
// CHECK-NEXT: OpDecorate [[func4_ii_plus_j:%[0-9]+]] NoContraction

void main() {
  float4 a, b, c, d;
  precise float4 v; 

// CHECK:      [[a_mul_b]] = OpFMul %v3float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:               OpStore %r [[a_mul_b]]
  float3 r = float3((float3)a * (float3)b); // precise, used to compute v.xyz


// CHECK:      [[c_mul_d]] = OpFMul %v3float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:               OpStore %s [[c_mul_d]]
  float3 s = float3((float3)c * (float3)d); // precise, used to compute v.xyz

// CHECK:                     OpLoad %v3float %r
// CHECK-NEXT:                OpLoad %v3float %s
// CHECK-NEXT: [[r_plus_s]] = OpFAdd %v3float {{%[0-9]+}} {{%[0-9]+}}
  v.xyz = r + s; // precise
  
// CHECK:                           OpAccessChain %_ptr_Function_float %a %int_3
// CHECK-NEXT:                      OpLoad %float
// CHECK-NEXT:                      OpAccessChain %_ptr_Function_float %b %int_3
// CHECK-NEXT:                      OpLoad %float
// CHECK-NEXT:      [[aw_mul_bw]] = OpFMul %float
// CHECK-NEXT:                      OpAccessChain %_ptr_Function_float %c %int_3
// CHECK-NEXT:                      OpLoad %float
// CHECK-NEXT:                      OpAccessChain %_ptr_Function_float %d %int_3
// CHECK-NEXT:                      OpLoad %float
// CHECK-NEXT:      [[cw_mul_dw]] = OpFMul %float
// CHECK-NEXT: [[awbw_plus_cwdw]] = OpFAdd %float
  v.w = (a.w * b.w) + (c.w * d.w);  // precise


  v.x = func(a.x, b.x, c.x, d.x);   // values computed in func() are NOT precise
  
  // Even though v.x is precise, values computed inside func2 are not forced to
  // be precise. Meaning, precise-ness does not cross function boundary.
  v.x = func2(a.x, b.x, c.x, d.x);
  
  // Even though v.x is precise, values computed inside func4 are not forced to
  // be precise. Meaning, precise-ness does not cross function boundary.
  v.x = func3(a.x, b.x, c.x, d.x);

// Because the precise keyword is on the output parameter in func4, the formal input parameters will get marked as precise as well. This should propagate to the actual parameters.
// CHECK: [[func_ax_mul_bx]] = OpFMul
// CHECK: OpStore %param_var_i [[func_ax_mul_bx]]
// CHECK: [[func_cx_mul_dx]] = OpFMul
// CHECK: OpStore %param_var_j [[func_cx_mul_dx]]
// CHECK: OpFunctionCall %float %func4 %param_var_i %param_var_j
  func4(a.x * b.x, c.x * d.x, v.x);
}

float func(float e, float f, float g, float h) {
  return (e*f) + (g*h); // no constraint on order or operator consistency
}

// CHECK: %func2 = OpFunction %float
float func2(float e, float f, float g, float h) {
// CHECK:                             OpLoad %float %e_0
// CHECK-NEXT:                        OpLoad %float %f_0
// CHECK-NEXT:    [[func2_e_mul_f]] = OpFMul %float
// CHECK-NEXT:                        OpLoad %float %g_0
// CHECK-NEXT:                        OpLoad %float %h_0
// CHECK-NEXT:    [[func2_g_mul_h]] = OpFMul %float
// CHECK-NEXT: [[func2_ef_plus_gh]] = OpFAdd %float
  precise float result = (e*f) + (g*h); // ensures same precision for the two multiplies
  return result;
}

// CHECK: %func3 = OpFunction %float
precise float func3(float e, float f, float g, float h) {
// CHECK:                             OpLoad %float %e_1
// CHECK-NEXT:                        OpLoad %float %f_1
// CHECK-NEXT:    [[func3_e_mul_f]] = OpFMul %float
// CHECK-NEXT:                        OpLoad %float %g_1
// CHECK-NEXT:                        OpLoad %float %h_1
// CHECK-NEXT:    [[func3_g_mul_h]] = OpFMul %float
// CHECK-NEXT: [[func3_ef_plus_gh]] = OpFAdd %float [[func3_e_mul_f]] [[func3_g_mul_h]]
  float result = (e*f) + (g*h); // precise because it's the function return value.
  return result;
}

// CHECK: %func4 = OpFunction %float
float func4(float i, float j, precise out float k) {
// CEHCK:                            OpLoad %float %i
// CEHCK-NEXT:                       OpLoad %float %i
// CEHCK-NEXT:   [[func4_i_mul_i]] = OpFMul %float
// CEHCK-NEXT:                       OpLoad %float %j
// CEHCK-NEXT: [[func4_ii_plus_j]] = OpFAdd %float
  k = i * i + j; // precise, due to <k> declaration
  return 1.0;
}

