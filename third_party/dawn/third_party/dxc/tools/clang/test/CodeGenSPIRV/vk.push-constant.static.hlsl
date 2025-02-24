// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S
{
  const static uint a = 1;
  uint b;
};

// CHECK:                                          OpMemberName %type_PushConstant_S 0 "b"
// CHECK:                   %type_PushConstant_S = OpTypeStruct %uint
// CHECK: %_ptr_PushConstant_type_PushConstant_S = OpTypePointer PushConstant %type_PushConstant_S

[[vk::push_constant]] S s;
// CHECK: %a = OpVariable %_ptr_Private_uint Private
// CHECK: %s = OpVariable %_ptr_PushConstant_type_PushConstant_S PushConstant

// CHECK: OpStore %a %uint_1

[numthreads(1,1,1)]
void main()
{
  uint32_t v = s.b;
// CHECK:               %v = OpVariable %_ptr_Function_uint Function
// CHECK:  [[ptr:%[0-9]+]] = OpAccessChain %_ptr_PushConstant_uint %s %int_0
// CHECK: [[load:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:                    OpStore %v [[load]]

  uint32_t w = S::a;
// CHECK:                    OpStore %w %uint_1

  uint32_t x = s.a;
// CHECK: [[load:%[0-9]+]] = OpLoad %uint %a
// CHECK:                    OpStore %x [[load]]
}
