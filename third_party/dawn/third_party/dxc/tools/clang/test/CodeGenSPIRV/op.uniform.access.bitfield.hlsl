// RUN: %dxc -T cs_6_0 -E main -spirv -fcgl %s

struct S {
  uint32_t a : 1;
  uint32_t b : 1;
};
// CHECK-DAG:                [[S:[a-zA-Z0-9_]+]] = OpTypeStruct %uint
// CHECK-DAG:            [[arr_S:[a-zA-Z0-9_]+]] = OpTypeRuntimeArray [[S]]
// CHECK-DAG:        [[type_buff:[a-zA-Z0-9_]+]] = OpTypeStruct [[arr_S]]
// CHECK-DAG: [[ptr_uint_uniform:[a-zA-Z0-9_]+]] = OpTypePointer Uniform %uint
// CHECK-DAG:         [[ptr_buff:[a-zA-Z0-9_]+]] = OpTypePointer Uniform [[type_buff]]

RWStructuredBuffer<S> rwbuffer;
// CHECK-DAG: %rwbuffer = OpVariable [[ptr_buff]] Uniform

[numthreads(1, 1, 1)]
void main() {
  uint32_t u = rwbuffer[2].b;
// CHECK:  [[ptr:%[0-9]+]] = OpAccessChain [[ptr_uint_uniform]] %rwbuffer %int_0 %uint_2 %int_0
// CHECK:  [[tmp:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[tmp2:%[0-9]+]] = OpBitFieldUExtract %uint [[tmp]] %uint_1 %uint_1
// CHECK:                    OpStore %u [[tmp2]]
}
