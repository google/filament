// RUN: %dxc -T vs_6_6 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

// CHECK: [[type_S:%[a-zA-Z0-9_]+]] = OpTypeStruct %uint %uint %uint
// CHECK: [[rarr_S:%[a-zA-Z0-9_]+]] = OpTypeRuntimeArray [[type_S]]
// CHECK: [[buffer:%[a-zA-Z0-9_]+]] = OpTypeStruct [[rarr_S]]
// CHECK: [[ptr_buffer:%[a-zA-Z0-9_]+]] = OpTypePointer Uniform [[buffer]]
struct S {
    uint f1;
    uint f2 : 1;
    uint f3 : 1;
    uint f4;
};

// CHECK: [[var_buffer:%[a-zA-Z0-9_]+]] = OpVariable [[ptr_buffer]] Uniform
StructuredBuffer<S> buffer;

void main(uint id : A) {
  // CHECK: [[id:%[0-9]+]] = OpLoad %uint %id
  // CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[var_buffer]] %int_0 [[id]] %int_1
  // CHECK: [[value:%[0-9]+]] = OpLoad %uint [[ptr]]
  // CHECK: [[value_0:%[0-9]+]] = OpBitFieldUExtract %uint [[value]] %uint_1 %uint_1
  // CHECK: OpStore %tmp [[value_0]]
  uint tmp = buffer[id].f3;
}
