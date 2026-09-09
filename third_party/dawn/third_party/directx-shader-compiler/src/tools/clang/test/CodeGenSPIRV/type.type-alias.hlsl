// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

using myInt = int;
using myConstUint = const uint;
using v4f = float4;
using m2v3f = float2x3;

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

// CHECK: %v1 = OpVariable %_ptr_Function_int Function
    myInt v1;
// CHECK: %v2 = OpVariable %_ptr_Function_uint Function
    myConstUint v2;
// CHECK: %v3 = OpVariable %_ptr_Function_v4float Function
    v4f v3;
// CHECK: %v4 = OpVariable %_ptr_Function_mat2v3float Function
    m2v3f v4;
}
