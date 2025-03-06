// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v3i0:%[0-9]+]] = OpConstantComposite %v3int %int_0 %int_0 %int_0
SamplerState gSS1;
SamplerState gSS2;

Texture2D gTex;

uint foo() { return 1; }
float bar() { return 3.0; }
uint zoo();

void main() {
  // CHECK-LABEL: %bb_entry = OpLabel

  // CHECK: %temp_var_ternary = OpVariable %_ptr_Function_mat2v3float Function
  // CHECK: %temp_var_ternary_0 = OpVariable %_ptr_Function_mat2v3float Function
  // CHECK: %temp_var_ternary_1 = OpVariable %_ptr_Function_type_sampler Function
  // CHECK: %temp_var_ternary_2 = OpVariable %_ptr_Function_type_sampler Function

  bool b0;
  int m, n, o;
  // Plain assign (scalar)
  // CHECK:      [[b0:%[0-9]+]] = OpLoad %bool %b0
  // CHECK-NEXT: [[m0:%[0-9]+]] = OpLoad %int %m
  // CHECK-NEXT: [[n0:%[0-9]+]] = OpLoad %int %n
  // CHECK-NEXT: [[s0:%[0-9]+]] = OpSelect %int [[b0]] [[m0]] [[n0]]
  // CHECK-NEXT: OpStore %o [[s0]]
  o = b0 ? m : n;

  bool1 b1;
  bool3 b3;
  uint1 p, q, r;
  float3 x, y, z;
  // Plain assign (vector)
  // CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %bool %b1
  // CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %uint %p
  // CHECK-NEXT: [[q0:%[0-9]+]] = OpLoad %uint %q
  // CHECK-NEXT: [[s1:%[0-9]+]] = OpSelect %uint [[b1]] [[p0]] [[q0]]
  // CHECK-NEXT: OpStore %r [[s1]]
  r = b1 ? p : q;
  // CHECK-NEXT: [[b3:%[0-9]+]] = OpLoad %v3bool %b3
  // CHECK-NEXT: [[x0:%[0-9]+]] = OpLoad %v3float %x
  // CHECK-NEXT: [[y0:%[0-9]+]] = OpLoad %v3float %y
  // CHECK-NEXT: [[s2:%[0-9]+]] = OpSelect %v3float [[b3]] [[x0]] [[y0]]
  // CHECK-NEXT: OpStore %z [[s2]]
  z = b3 ? x : y;

  // Try condition with various type.
  // Note: the SPIR-V OpSelect selection argument must be the same size as the return type.
  int3 u, v, w;
  float2x3 umat, vmat, wmat;
  bool cond;
  bool3 cond3;
  float floatCond;
  int intCond;
  int3 int3Cond;

  // CHECK:      [[cond3:%[0-9]+]] = OpLoad %v3bool %cond3
  // CHECK-NEXT:     [[u:%[0-9]+]] = OpLoad %v3int %u
  // CHECK-NEXT:     [[v:%[0-9]+]] = OpLoad %v3int %v
  // CHECK-NEXT:       {{%[0-9]+}} = OpSelect %v3int [[cond3]] [[u]] [[v]]
  w = cond3 ? u : v;

  // CHECK:       [[cond:%[0-9]+]] = OpLoad %bool %cond
  // CHECK-NEXT: [[splat:%[0-9]+]] = OpCompositeConstruct %v3bool [[cond]] [[cond]] [[cond]]
  // CHECK-NEXT:     [[u_0:%[0-9]+]] = OpLoad %v3int %u
  // CHECK-NEXT:     [[v_0:%[0-9]+]] = OpLoad %v3int %v
  // CHECK-NEXT:       {{%[0-9]+}} = OpSelect %v3int [[splat]] [[u_0]] [[v_0]]
  w = cond ? u : v;

  // CHECK:      [[floatCond:%[0-9]+]] = OpLoad %float %floatCond
  // CHECK-NEXT:  [[boolCond:%[0-9]+]] = OpFOrdNotEqual %bool [[floatCond]] %float_0
  // CHECK-NEXT: [[bool3Cond:%[0-9]+]] = OpCompositeConstruct %v3bool [[boolCond]] [[boolCond]] [[boolCond]]
  // CHECK-NEXT:         [[u_1:%[0-9]+]] = OpLoad %v3int %u
  // CHECK-NEXT:         [[v_1:%[0-9]+]] = OpLoad %v3int %v
  // CHECK-NEXT:           {{%[0-9]+}} = OpSelect %v3int [[bool3Cond]] [[u_1]] [[v_1]]
  w = floatCond ? u : v;

  // CHECK:       [[int3Cond:%[0-9]+]] = OpLoad %v3int %int3Cond
  // CHECK-NEXT: [[bool3Cond_0:%[0-9]+]] = OpINotEqual %v3bool [[int3Cond]] [[v3i0]]
  // CHECK-NEXT:         [[u_2:%[0-9]+]] = OpLoad %v3int %u
  // CHECK-NEXT:         [[v_2:%[0-9]+]] = OpLoad %v3int %v
  // CHECK-NEXT:           {{%[0-9]+}} = OpSelect %v3int [[bool3Cond_0]] [[u_2]] [[v_2]]
  w = int3Cond ? u : v;

  // CHECK:       [[intCond:%[0-9]+]] = OpLoad %int %intCond
  // CHECK-NEXT: [[boolCond_0:%[0-9]+]] = OpINotEqual %bool [[intCond]] %int_0
  // CHECK-NEXT:     [[umat:%[0-9]+]] = OpLoad %mat2v3float %umat
  // CHECK-NEXT:     [[vmat:%[0-9]+]] = OpLoad %mat2v3float %vmat
  // CHECK-NEXT:                     OpSelectionMerge %if_merge None
  // CHECK-NEXT:                     OpBranchConditional [[boolCond_0]] %if_true %if_false
  // CHECK-NEXT:          %if_true = OpLabel
  // CHECK-NEXT:                     OpStore %temp_var_ternary [[umat]]
  // CHECK-NEXT:                     OpBranch %if_merge
  // CHECK-NEXT:         %if_false = OpLabel
  // CHECK-NEXT:                     OpStore %temp_var_ternary [[vmat]]
  // CHECK-NEXT:                     OpBranch %if_merge
  // CHECK-NEXT:         %if_merge = OpLabel
  // CHECK-NEXT:  [[tempVar:%[0-9]+]] = OpLoad %mat2v3float %temp_var_ternary
  // CHECK-NEXT:                     OpStore %wmat [[tempVar]]
  wmat = intCond ? umat : vmat;

  // Make sure literal types are handled correctly in ternary ops

  // CHECK: [[b_float:%[0-9]+]] = OpSelect %float {{%[0-9]+}} %float_1_5 %float_2_5
  // CHECK-NEXT:    {{%[0-9]+}} = OpConvertFToS %int [[b_float]]
  int b = cond ? 1.5 : 2.5;

  // CHECK:      [[a_int:%[0-9]+]] = OpSelect %int {{%[0-9]+}} %int_1 %int_0
  // CHECK-NEXT:       {{%[0-9]+}} = OpConvertSToF %float [[a_int]]
  float a = cond ? 1 : 0;

  // CHECK:      [[c_long:%[0-9]+]] = OpSelect %long {{%[0-9]+}} %long_3000000000 %long_4000000000
  double c = cond ? 3000000000 : 4000000000;

  // CHECK:      [[d_int:%[0-9]+]] = OpSelect %int {{%[0-9]+}} %int_1 %int_0
  // CHECK:      {{%[0-9]+}} = OpBitcast %uint [[d_int]]
  uint d = cond ? 1 : 0;

  float2x3 e;
  float2x3 f;
  // CHECK:     [[cond_0:%[0-9]+]] = OpLoad %bool %cond
  // CHECK-NEXT:   [[e:%[0-9]+]] = OpLoad %mat2v3float %e
  // CHECK-NEXT:   [[f:%[0-9]+]] = OpLoad %mat2v3float %f
  // CHECK-NEXT:                OpSelectionMerge %if_merge_0 None
  // CHECK-NEXT:                OpBranchConditional [[cond_0]] %if_true_0 %if_false_0
  // CHECK-NEXT:   %if_true_0 = OpLabel
  // CHECK-NEXT:                OpStore %temp_var_ternary_0 [[e]]
  // CHECK-NEXT:                OpBranch %if_merge_0
  // CHECK-NEXT:  %if_false_0 = OpLabel
  // CHECK-NEXT:                OpStore %temp_var_ternary_0 [[f]]
  // CHECK-NEXT:                OpBranch %if_merge_0
  // CHECK-NEXT:  %if_merge_0 = OpLabel
  // CHECK-NEXT:[[temp:%[0-9]+]] = OpLoad %mat2v3float %temp_var_ternary_0
  // CHECK-NEXT:                OpStore %g [[temp]]
  float2x3 g = cond ? e : f;

  // CHECK:       [[inner:%[0-9]+]] = OpSelect %int {{%[0-9]+}} %int_1 %int_2
  // CHECK-NEXT: [[outter:%[0-9]+]] = OpSelect %int {{%[0-9]+}} %int_9 [[inner]]
  // CHECK-NEXT:        {{%[0-9]+}} = OpBitcast %uint [[outter]]
  uint h = cond ? 9 : (cond ? 1 : 2);

  //CHECK:      [[i_int:%[0-9]+]] = OpSelect %int {{%[0-9]+}} %int_1 %int_0
  //CHECK-NEXT:       {{%[0-9]+}} = OpINotEqual %bool [[i_int]] %int_0
  bool i = cond ? 1 : 0;

  // CHECK:     [[foo:%[0-9]+]] = OpFunctionCall %uint %foo
  // CHECKNEXT:     {{%[0-9]+}} = OpSelect %uint {{%[0-9]+}} %uint_3 [[foo]]
  uint j = cond ? 3 : foo();

  // CHECK:          [[bar:%[0-9]+]] = OpFunctionCall %float %bar
  // CHECK-NEXT: [[k_float:%[0-9]+]] = OpSelect %float {{%[0-9]+}} %float_4 [[bar]]
  // CHECK-NEXT:         {{%[0-9]+}} = OpConvertFToU %uint [[k_float]]
  uint k = cond ? 4 : bar();

  // AST looks like:
  // |-ConditionalOperator 'SamplerState'
  // | |-DeclRefExpr 'bool' lvalue Var 0x1476949e328 'cond' 'bool'
  // | |-DeclRefExpr 'SamplerState' lvalue Var 0x1476742e498 'gSS1' 'SamplerState'
  // | `-DeclRefExpr 'SamplerState' lvalue Var 0x1476742e570 'gSS2' 'SamplerState'

  // CHECK:      [[cond_1:%[0-9]+]] = OpLoad %bool %cond
  // CHECK-NEXT: [[gSS1:%[0-9]+]] = OpLoad %type_sampler %gSS1
  // CHECK-NEXT: [[gSS2:%[0-9]+]] = OpLoad %type_sampler %gSS2
  // CHECK-NEXT:                 OpSelectionMerge %if_merge_1 None
  // CHECK-NEXT:                 OpBranchConditional [[cond_1]] %if_true_1 %if_false_1
  // CHECK-NEXT:    %if_true_1 = OpLabel
  // CHECK-NEXT:                 OpStore %temp_var_ternary_1 [[gSS1]]
  // CHECK-NEXT:                 OpBranch %if_merge_1
  // CHECK-NEXT:   %if_false_1 = OpLabel
  // CHECK-NEXT:                 OpStore %temp_var_ternary_1 [[gSS2]]
  // CHECK-NEXT:                 OpBranch %if_merge_1
  // CHECK-NEXT:   %if_merge_1 = OpLabel
  // CHECK-NEXT:   [[ss:%[0-9]+]] = OpLoad %type_sampler %temp_var_ternary_1
  // CHECK-NEXT:      {{%[0-9]+}} = OpSampledImage %type_sampled_image {{%[0-9]+}} [[ss]]
  float4 l = gTex.Sample(cond ? gSS1 : gSS2, float2(1., 2.));

  zoo();

// CHECK:       [[cond2x3:%[0-9]+]] = OpLoad %_arr_v3bool_uint_2 %cond2x3
// CHECK-NEXT:  [[true2x3:%[0-9]+]] = OpLoad %mat2v3float %true2x3
// CHECK-NEXT: [[false2x3:%[0-9]+]] = OpLoad %mat2v3float %false2x3
// CHECK-NEXT:       [[c0:%[0-9]+]] = OpCompositeExtract %v3bool [[cond2x3]] 0
// CHECK-NEXT:       [[t0:%[0-9]+]] = OpCompositeExtract %v3float [[true2x3]] 0
// CHECK-NEXT:       [[f0:%[0-9]+]] = OpCompositeExtract %v3float [[false2x3]] 0
// CHECK-NEXT:       [[r0:%[0-9]+]] = OpSelect %v3float [[c0]] [[t0]] [[f0]]
// CHECK-NEXT:       [[c1:%[0-9]+]] = OpCompositeExtract %v3bool [[cond2x3]] 1
// CHECK-NEXT:       [[t1:%[0-9]+]] = OpCompositeExtract %v3float [[true2x3]] 1
// CHECK-NEXT:       [[f1:%[0-9]+]] = OpCompositeExtract %v3float [[false2x3]] 1
// CHECK-NEXT:       [[r1:%[0-9]+]] = OpSelect %v3float [[c1]] [[t1]] [[f1]]
// CHECK-NEXT:   [[result:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[r0]] [[r1]]
// CHECK-NEXT:                     OpStore %result2x3 [[result]]
  bool2x3 cond2x3;
  float2x3 true2x3, false2x3;
  float2x3 result2x3 = cond2x3 ? true2x3 : false2x3;

  // Note that AST does not have a ImplicitCastExpr around intCond for this case:
  // | `-VarDecl 0x2a90094e0d0 <col:3, col:37> col:16 s 'SamplerState' cinit
  // |   `-ConditionalOperator 0x2a90094e1b8 <col:20, col:37> 'SamplerState'
  // |     |-DeclRefExpr 0x2a90094e140 <col:20> 'int' lvalue Var 0x2a90051e1c0 'intCond' 'int'
  // |     |-DeclRefExpr 0x2a90094e168 <col:30> 'SamplerState' lvalue Var 0x2a9004c6f40 'gSS1' 'SamplerState'
  // |     `-DeclRefExpr 0x2a90094e190 <col:37> 'SamplerState' lvalue Var 0x2a9004c7000 'gSS2' 'SamplerState'

  // CHECK:       [[intCond_0:%[0-9]+]] = OpLoad %int %intCond
  // CHECK-NEXT:     [[gSS1_0:%[0-9]+]] = OpLoad %type_sampler %gSS1
  // CHECK-NEXT:     [[gSS2_0:%[0-9]+]] = OpLoad %type_sampler %gSS2
  // CHECK-NEXT: [[boolCond_1:%[0-9]+]] = OpINotEqual %bool [[intCond_0]] %int_0
  // CHECK-NEXT:                     OpSelectionMerge %if_merge_2 None
  // CHECK-NEXT:                     OpBranchConditional [[boolCond_1]] %if_true_2 %if_false_2
  // CHECK-NEXT:        %if_true_2 = OpLabel
  // CHECK-NEXT:                     OpStore %temp_var_ternary_2 [[gSS1_0]]
  // CHECK-NEXT:                     OpBranch %if_merge_2
  // CHECK-NEXT:       %if_false_2 = OpLabel
  // CHECK-NEXT:                     OpStore %temp_var_ternary_2 [[gSS2_0]]
  // CHECK-NEXT:                     OpBranch %if_merge_2
  // CHECK-NEXT:       %if_merge_2 = OpLabel
  // CHECK-NEXT:  [[tempVar_0:%[0-9]+]] = OpLoad %type_sampler %temp_var_ternary_2
  // CHECK-NEXT:                     OpStore %s [[tempVar_0]]
  SamplerState s = intCond ? gSS1 : gSS2;
}

//
// The literal integer type should be deduced from the function return type.
//
// CHECK:      [[result_0:%[0-9]+]] = OpSelect %int {{%[0-9]+}} %int_1 %int_2
// CHECK-NEXT:                   OpBitcast %uint [[result_0]]
uint zoo() {
  bool cond;
  return cond ? 1 : 2;
}

