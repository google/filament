// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct T {
    float2 val[3];
};

// CHECK: OpName %type_PushConstant_S "type.PushConstant.S"
// CHECK: OpMemberName %type_PushConstant_S 0 "f1"
// CHECK: OpMemberName %type_PushConstant_S 1 "f2"
// CHECK: OpMemberName %type_PushConstant_S 2 "f3"
// CHECK: OpMemberName %type_PushConstant_S 3 "f4"

// CHECK: %type_PushConstant_S = OpTypeStruct %float %v3float %mat2v3float %T
struct S {
    float    f1;
    float3   f2;
    float2x3 f3;
    T        f4;
};
// CHECK: %_ptr_PushConstant_type_PushConstant_S = OpTypePointer PushConstant %type_PushConstant_S

// CHECK: %pcs = OpVariable %_ptr_PushConstant_type_PushConstant_S PushConstant
[[vk::push_constant]]
S pcs;

float main() : A {
    return
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_PushConstant_float %pcs %int_0
        pcs.f1 +
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_PushConstant_v3float %pcs %int_1
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_PushConstant_float [[ptr]] %int_2
        pcs.f2.z +
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_PushConstant_float %pcs %int_2 %uint_1 %uint_2
        pcs.f3[1][2] +
// CHECK: [[base:%[0-9]+]] = OpAccessChain %_ptr_PushConstant__arr_v2float_uint_3 %pcs %int_3 %int_0
// CHECK: [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_PushConstant_v2float [[base]] %int_2
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_PushConstant_float [[ptr_0]] %int_1
        pcs.f4.val[2].y;
}
