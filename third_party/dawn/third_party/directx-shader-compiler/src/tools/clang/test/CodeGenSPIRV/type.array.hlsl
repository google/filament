// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %_arr_uint_uint_4 = OpTypeArray %uint %uint_4

// CHECK: %_arr_int_uint_8 = OpTypeArray %int %uint_8

// CHECK: %_arr_float_uint_4 = OpTypeArray %float %uint_4
// CHECK: %_arr__arr_float_uint_4_uint_8 = OpTypeArray %_arr_float_uint_4 %uint_8

void main() {
    const uint size = 4 * 3 - 4;

// CHECK: %x = OpVariable %_ptr_Function__arr_uint_uint_4 Function
    uint  x[4];
// CHECK: %y = OpVariable %_ptr_Function__arr_int_uint_8 Function
    int   y[size];
// CHECK: %z = OpVariable %_ptr_Function__arr__arr_float_uint_4_uint_8 Function
    float z[size][4];
}
