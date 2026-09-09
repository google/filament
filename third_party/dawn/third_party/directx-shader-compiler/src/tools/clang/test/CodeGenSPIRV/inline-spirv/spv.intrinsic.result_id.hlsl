// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

[[vk::ext_instruction(/* OpLoad */ 61)]]
vk::ext_result_id<float> load([[vk::ext_reference]] float pointer,
                              [[vk::ext_literal]] int memoryOperands);

[[vk::ext_instruction(/* OpStore */ 62)]]
void store([[vk::ext_reference]] float pointer,
           vk::ext_result_id<float> value,
           [[vk::ext_literal]] int memoryOperands);

[numthreads(1,1,1)]
void main() {
  float foo, bar;

//CHECK: [[foo_value:%[a-zA-Z0-9_]+]] = OpLoad %float %foo None
//CHECK:                      OpStore %bar [[foo_value]] Volatile
  vk::ext_result_id<float> foo_value = load(foo, /* None */ 0x0);
  store(bar, foo_value, /* Volatile */ 0x1);
}
