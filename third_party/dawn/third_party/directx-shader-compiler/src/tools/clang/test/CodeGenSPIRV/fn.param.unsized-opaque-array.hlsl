// RUN: %dxc -T ps_6_0 -E main -Vd -fcgl  %s -spirv | FileCheck %s

struct PSInput
{
    float4 color : COLOR;
};

Texture2D bindless[];

sampler DummySampler;

// CHECK: %_ptr_Function__ptr_UniformConstant__runtimearr_type_2d_image = OpTypePointer Function %_ptr_UniformConstant__runtimearr_type_2d_image
// CHECK: %param_var_src = OpVariable %_ptr_Function__ptr_UniformConstant__runtimearr_type_2d_image Function
// CHECK:                OpStore %param_var_src %bindless
// CHECK:                OpFunctionCall
// CHECK:         %src = OpFunctionParameter %_ptr_Function__ptr_UniformConstant__runtimearr_type_2d_image
// CHECK: [[src:%[0-9]+]] = OpLoad %_ptr_UniformConstant__runtimearr_type_2d_image %src
// CHECK: [[idx:%[0-9]+]] = OpLoad %uint %index
// CHECK:                OpAccessChain %_ptr_Function_type_2d_image [[src]] [[idx]]

float4 SampleArray(Texture2D src[], uint index, float2 uv)
{
    return src[index].Sample(DummySampler, uv);
}

float4 main(PSInput input) : SV_TARGET
{
    return input.color * SampleArray(bindless, 4, float2(1,1));
}
