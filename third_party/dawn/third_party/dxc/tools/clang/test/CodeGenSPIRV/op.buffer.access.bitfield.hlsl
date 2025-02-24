// RUN: %dxc -T ps_6_6 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

struct S1 {
    uint f1 : 1;
    uint f2 : 1;
};

Buffer<S1> input_1;

void main() {
// CHECK: [[img:%[0-9]+]] = OpLoad %type_buffer_image %input_1
// CHECK: [[tmp:%[0-9]+]] = OpImageFetch %v4uint [[img]] %uint_0 None
// CHECK: [[tmp_0:%[0-9]+]] = OpVectorShuffle %v2uint [[tmp]] [[tmp]] 0 1
// CHECK: [[tmp_f1:%[0-9]+]] = OpCompositeExtract %uint [[tmp_0]] 0
// CHECK: [[tmp_s1:%[0-9]+]] = OpCompositeConstruct %S1 [[tmp_f1]]
// CHECK: OpStore [[tmp_var_S1:%[a-zA-Z0-9_]+]] [[tmp_s1]]
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint [[tmp_var_S1]] %int_0
// CHECK: [[load:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[extract:%[0-9]+]] = OpBitFieldUExtract %uint [[load]] %uint_1 %uint_1
// CHECK: OpStore %tmp [[extract]]
  uint tmp = input_1[0].f2;
}
