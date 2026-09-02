// RUN: %dxc -T cs_6_0 -E main -spirv -fcgl %s

struct S {
  uint32_t a : 1;
  uint32_t b : 1;
};
// CHECK-DAG:                [[S:[a-zA-Z0-9_]+]] = OpTypeStruct %uint
// CHECK-DAG:            [[ptr_S:[a-zA-Z0-9_]+]] = OpTypePointer PushConstant [[S]]
// CHECK-DAG:         [[ptr_uint:[a-zA-Z0-9_]+]] = OpTypePointer PushConstant %uint

[[vk::push_constant]] S buffer;
// CHECK-DAG:   %buffer = OpVariable [[ptr_S]] PushConstant

[numthreads(1, 1, 1)]
void main() {
  uint32_t v = buffer.b;
// CHECK:  [[ptr:%[0-9]+]] = OpAccessChain [[ptr_uint]] %buffer %int_0
// CHECK:  [[tmp:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[tmp2:%[0-9]+]] = OpBitFieldUExtract %uint [[tmp]] %uint_1 %uint_1
// CHECK:                    OpStore %v [[tmp2]]
}
