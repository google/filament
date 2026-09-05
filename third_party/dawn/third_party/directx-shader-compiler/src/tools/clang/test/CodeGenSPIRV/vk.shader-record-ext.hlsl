// RUN: %dxc -T lib_6_3 -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

struct T {
    float2 val[3];
};

// CHECK: OpName %type_ConstantBuffer_S "type.ConstantBuffer.S"
// CHECK: OpMemberName %type_ConstantBuffer_S 0 "f1"
// CHECK: OpMemberName %type_ConstantBuffer_S 1 "f2"
// CHECK: OpMemberName %type_ConstantBuffer_S 2 "f3"
// CHECK: OpMemberName %type_ConstantBuffer_S 3 "f4"
// CHECK-NOT: OpDecorate %srb DescriptorSet
// CHECK-NOT: OpDecorate %srb Binding

// CHECK: %type_ConstantBuffer_S = OpTypeStruct %float %v3float %mat2v3float %T
struct S {
    float    f1;
    float3   f2;
    float2x3 f3;
    T        f4;
};
// CHECK: %_ptr_ShaderRecordBufferKHR_type_ConstantBuffer_S = OpTypePointer ShaderRecordBufferKHR %type_ConstantBuffer_S

// CHECK: %srb = OpVariable %_ptr_ShaderRecordBufferKHR_type_ConstantBuffer_S ShaderRecordBufferKHR
[[vk::shader_record_ext]]
ConstantBuffer<S> srb;

struct Payload { float p; };
struct Attribute { float a; };

[shader("miss")]
void main(inout Payload P)
{
   P.p =
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_ShaderRecordBufferKHR_float %srb %int_0
        srb.f1 +
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_ShaderRecordBufferKHR_v3float %srb %int_1
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_ShaderRecordBufferKHR_float [[ptr]] %int_2
        srb.f2.z +
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_ShaderRecordBufferKHR_float %srb %int_2 %uint_1 %uint_2
        srb.f3[1][2] +
// CHECK: [[base:%[0-9]+]] = OpAccessChain %_ptr_ShaderRecordBufferKHR__arr_v2float_uint_3 %srb %int_3 %int_0
// CHECK: [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_ShaderRecordBufferKHR_v2float [[base]] %int_2
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_ShaderRecordBufferKHR_float [[ptr_0]] %int_1
        srb.f4.val[2].y;
}
