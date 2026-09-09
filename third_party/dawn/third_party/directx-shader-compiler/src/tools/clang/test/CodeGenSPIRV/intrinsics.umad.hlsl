// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate [[mul1:%[0-9]+]] NoContraction
// CHECK: OpDecorate [[add1:%[0-9]+]] NoContraction
// CHECK: OpDecorate [[mul2:%[0-9]+]] NoContraction
// CHECK: OpDecorate [[add2:%[0-9]+]] NoContraction
// CHECK: OpDecorate [[mul3:%[0-9]+]] NoContraction
// CHECK: OpDecorate [[add3:%[0-9]+]] NoContraction
// CHECK: OpDecorate [[mul4:%[0-9]+]] NoContraction
// CHECK: OpDecorate [[add4:%[0-9]+]] NoContraction

void main() {
  uint    d1, d2, d3;
  uint4   e1, e2, e3;
  uint2x3 f1, f2, f3;
  int fma_d;
  int4 fma_e;
  int2x3 fma_f;

// CHECK:       [[d1:%[0-9]+]] = OpLoad %uint %d1
// CHECK-NEXT:  [[d2:%[0-9]+]] = OpLoad %uint %d2
// CHECK-NEXT:  [[d3:%[0-9]+]] = OpLoad %uint %d3
// CHECK-NEXT:     [[mul1]] = OpIMul %uint [[d1]] [[d2]]
// CHECK-NEXT:     [[add1]] = OpIAdd %uint [[mul1]] [[d3]]
// CHECK-NEXT:     {{%[0-9]+}} = OpBitcast %int [[add1]]
  fma_d = mad(d1, d2, d3);

// CHECK:       [[e1:%[0-9]+]] = OpLoad %v4uint %e1
// CHECK-NEXT:  [[e2:%[0-9]+]] = OpLoad %v4uint %e2
// CHECK-NEXT:  [[e3:%[0-9]+]] = OpLoad %v4uint %e3
// CHECK-NEXT:     [[mul2]] = OpIMul %v4uint [[e1]] [[e2]]
// CHECK-NEXT:     [[add2]] = OpIAdd %v4uint [[mul2]] [[e3]]
// CHECK-NEXT:     {{%[0-9]+}} = OpBitcast %v4int [[add2]]
  fma_e = mad(e1, e2, e3);

// CHECK:           [[f1:%[0-9]+]] = OpLoad %_arr_v3uint_uint_2 %f1
// CHECK-NEXT:      [[f2:%[0-9]+]] = OpLoad %_arr_v3uint_uint_2 %f2
// CHECK-NEXT:      [[f3:%[0-9]+]] = OpLoad %_arr_v3uint_uint_2 %f3
// CHECK-NEXT:  [[f1row0:%[0-9]+]] = OpCompositeExtract %v3uint [[f1]] 0
// CHECK-NEXT:  [[f2row0:%[0-9]+]] = OpCompositeExtract %v3uint [[f2]] 0
// CHECK-NEXT:  [[f3row0:%[0-9]+]] = OpCompositeExtract %v3uint [[f3]] 0
// CHECK-NEXT:         [[mul3]] = OpIMul %v3uint [[f1row0]] [[f2row0]]
// CHECK-NEXT:         [[add3]] = OpIAdd %v3uint [[mul3]] [[f3row0]]
// CHECK-NEXT:  [[f1row1:%[0-9]+]] = OpCompositeExtract %v3uint [[f1]] 1
// CHECK-NEXT:  [[f2row1:%[0-9]+]] = OpCompositeExtract %v3uint [[f2]] 1
// CHECK-NEXT:  [[f3row1:%[0-9]+]] = OpCompositeExtract %v3uint [[f3]] 1
// CHECK-NEXT:         [[mul4]] = OpIMul %v3uint [[f1row1]] [[f2row1]]
// CHECK-NEXT:         [[add4]] = OpIAdd %v3uint [[mul4]] [[f3row1]]
// CHECK-NEXT:     [[mat:%[0-9]+]] = OpCompositeConstruct %_arr_v3uint_uint_2 [[add3]] [[add4]]
// CHECK-NEXT: [[matrow0:%[0-9]+]] = OpCompositeExtract %v3uint [[mat]] 0
// CHECK-NEXT:[[umatrow0:%[0-9]+]] = OpBitcast %v3int [[matrow0]]
// CHECK-NEXT: [[matrow1:%[0-9]+]] = OpCompositeExtract %v3uint [[mat]] 1
// CHECK-NEXT:[[umatrow1:%[0-9]+]] = OpBitcast %v3int [[matrow1]]
// CHECK-NEXT:         {{%[0-9]+}} = OpCompositeConstruct %_arr_v3int_uint_2 [[umatrow0]] [[umatrow1]]

  fma_f = mad(f1, f2, f3);
}

