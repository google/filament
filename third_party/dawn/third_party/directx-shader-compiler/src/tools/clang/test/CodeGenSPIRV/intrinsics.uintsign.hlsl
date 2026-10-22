// RUN: %dxc -T vs_6_0 -E main -fcgl -Vd  %s -spirv | FileCheck %s

// CHECK-DAG: %int_0 = OpConstant %int 0
// CHECK-DAG: %int_1 = OpConstant %int 1
// CHECK-DAG: %uint_0 = OpConstant %uint 0
// CHECK-DAG: %v3int = OpTypeVector %int 3
// CHECK-DAG: %v3uint = OpTypeVector %uint 3
// CHECK-DAG: [[zeros_uint3:%[0-9]+]] = OpConstantComposite %v3uint %uint_0 %uint_0 %uint_0
// CHECK-DAG: [[zeros_int3:%[0-9]+]] = OpConstantComposite %v3int %int_0 %int_0 %int_0
// CHECK-DAG: [[ones_int3:%[0-9]+]] = OpConstantComposite %v3int %int_1 %int_1 %int_1

void main() {
  int result;
  int3 result3;
  int3x3 result3x3;

// CHECK: [[a:%[0-9]+]] = OpLoad %uint %a
// CHECK-NEXT: [[cmp_a:%[0-9]+]] = OpUGreaterThan %bool [[a]] %uint_0
// CHECK-NEXT: [[select_a:%[0-9]+]] = OpSelect %int [[cmp_a]] %int_1 %int_0
// CHECK-NEXT: OpStore %result [[select_a]]
  uint a;
  result = sign(a);

// CHECK: [[b:%[0-9]+]] = OpLoad %uint %b
// CHECK-NEXT: [[cmp_b:%[0-9]+]] = OpUGreaterThan %bool [[b]] %uint_0
// CHECK-NEXT: [[select_b:%[0-9]+]] = OpSelect %int [[cmp_b]] %int_1 %int_0
// CHECK-NEXT: OpStore %result [[select_b]]
  uint1 b;
  result = sign(b);

// CHECK: [[c:%[0-9]+]] = OpLoad %v3uint %c
// CHECK-NEXT: [[cmp_c:%[0-9]+]] = OpUGreaterThan %v3bool [[c]] [[zeros_uint3]]
// CHECK-NEXT: [[select_c:%[0-9]+]] = OpSelect %v3int [[cmp_c]] [[ones_int3]] [[zeros_int3]]
// CHECK-NEXT: OpStore %result3 [[select_c]]
  uint3 c;
  result3 = sign(c);


// CHECK: [[d:%[0-9]+]] = OpLoad %uint %d
// CHECK-NEXT: [[cmp_d:%[0-9]+]] = OpUGreaterThan %bool [[d]] %uint_0
// CHECK-NEXT: [[select_d:%[0-9]+]] = OpSelect %int [[cmp_d]] %int_1 %int_0
// CHECK-NEXT: OpStore %result [[select_d]]
  uint1x1 d;
  result = sign(d);

// CHECK: [[e:%[0-9]+]] = OpLoad %v3uint %e
// CHECK-NEXT: [[cmp_e:%[0-9]+]] = OpUGreaterThan %v3bool [[e]] [[zeros_uint3]]
// CHECK-NEXT: [[select_e:%[0-9]+]] = OpSelect %v3int [[cmp_e]] [[ones_int3]] [[zeros_int3]]
// CHECK-NEXT: OpStore %result3 [[select_e]]
  uint1x3 e;
  result3 = sign(e);

// CHECK: [[f:%[0-9]+]] = OpLoad %v3uint %f
// CHECK-NEXT: [[cmp_f:%[0-9]+]] = OpUGreaterThan %v3bool [[f]] [[zeros_uint3]]
// CHECK-NEXT: [[select_f:%[0-9]+]] = OpSelect %v3int [[cmp_f]] [[ones_int3]] [[zeros_int3]]
// CHECK-NEXT: OpStore %result3 [[select_f]]
  uint3x1 f;
  result3 = sign(f);

// CHECK: [[h:%[0-9]+]] = OpLoad %_arr_v3uint_uint_3 %h
// CHECK-NEXT: [[h_row0:%[0-9]+]] = OpCompositeExtract %v3uint [[h]] 0
// CHECK-NEXT: [[cmp_h_row0:%[0-9]+]] = OpUGreaterThan %v3bool [[h_row0]] [[zeros_uint3]]
// CHECK-NEXT: [[select_h_row0:%[0-9]+]] = OpSelect %v3int [[cmp_h_row0]] [[ones_int3]] [[zeros_int3]]
// CHECK-NEXT: [[h_row1:%[0-9]+]] = OpCompositeExtract %v3uint [[h]] 1
// CHECK-NEXT: [[cmp_h_row1:%[0-9]+]] = OpUGreaterThan %v3bool [[h_row1]] [[zeros_uint3]]
// CHECK-NEXT: [[select_h_row1:%[0-9]+]] = OpSelect %v3int [[cmp_h_row1]] [[ones_int3]] [[zeros_int3]]
// CHECK-NEXT: [[h_row2:%[0-9]+]] = OpCompositeExtract %v3uint [[h]] 2
// CHECK-NEXT: [[cmp_h_row2:%[0-9]+]] = OpUGreaterThan %v3bool [[h_row2]] [[zeros_uint3]]
// CHECK-NEXT: [[select_h_row2:%[0-9]+]] = OpSelect %v3int [[cmp_h_row2]] [[ones_int3]] [[zeros_int3]]
// CHECK-NEXT: [[select_h:%[0-9]+]] = OpCompositeConstruct %_arr_v3uint_uint_3 [[select_h_row0]] [[select_h_row1]] [[select_h_row2]]
// CHECK-NEXT: OpStore %result3x3 [[select_h]]
  uint3x3 h;
  result3x3 = sign(h);
}
