// RUN: %dxc -T cs_6_0 -E main -fcgl -spirv %s | FileCheck %s

// CHECK-DAG:                           %PTE = OpTypeStruct %uint
// CHECK-DAG:               %_runtimearr_PTE = OpTypeRuntimeArray %PTE
// CHECK-DAG:                                  OpDecorate %_runtimearr_PTE ArrayStride 4
// CHECK-DAG: [[buffer_type:%[a-zA-Z0-9_]+]] = OpTypeStruct %_runtimearr_PTE
// CHECK-DAG:  [[buffer_ptr:%[a-zA-Z0-9_]+]] = OpTypePointer Uniform [[buffer_type]]
struct PTE {
  uint valid : 1;
  uint dirty : 1;
};

RWStructuredBuffer<PTE>  bad;
// CHECK:   %bad = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_PTE Uniform

[numthreads(1, 1, 1)]
void main() {
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %bad %int_0 %uint_1 %int_0
// CHECK: [[tmp:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[bit:%[0-9]+]] = OpBitFieldInsert %uint [[tmp]] %uint_1 %uint_1 %uint_1
// CHECK:                   OpStore [[ptr]] [[bit]]
  bad[1].dirty = 1;
}
