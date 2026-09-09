// RUN: %dxc -T cs_6_2 -E main -spirv -fcgl -enable-16bit-types %s | FileCheck %s

struct S1
{
  uint16_t a : 8;
};

S1 foo()
{
  return (S1)0;
}

[numthreads(1, 1, 1)]
void main() {
  uint16_t test = foo().a;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ushort %temp_var_S1 %int_0
// CHECK: [[raw:%[0-9]+]] = OpLoad %ushort [[ptr]]
// CHECK: [[tmp:%[0-9]+]] = OpShiftLeftLogical %ushort [[raw]] %uint_8
// CHECK: [[out:%[0-9]+]] = OpShiftRightLogical %ushort [[tmp]] %uint_8
// CHECK-NOT:               OpLoad %ushort [[out]]
// CHECK:                   OpStore %test [[out]]
}
