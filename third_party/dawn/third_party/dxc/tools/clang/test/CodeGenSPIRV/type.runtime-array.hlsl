// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability RuntimeDescriptorArray

// CHECK: OpExtension "SPV_EXT_descriptor_indexing"

struct S {
    float4 a;
    float3 b;
    float  c;
};

// CHECK: %type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknow
// CHECK: %_runtimearr_type_2d_image = OpTypeRuntimeArray %type_2d_image

// CHECK: %type_sampler = OpTypeSampler
// CHECK: %_runtimearr_type_sampler = OpTypeRuntimeArray %type_sampler

// CHECK: %type_ConstantBuffer_S = OpTypeStruct %v4float %v3float %float
// CHECK: %_runtimearr_type_ConstantBuffer_S = OpTypeRuntimeArray %type_ConstantBuffer_S

// CHECK: %MyTextures = OpVariable %_ptr_UniformConstant__runtimearr_type_2d_image UniformConstant
Texture2D<float4> MyTextures[];
// CHECK: %MySamplers = OpVariable %_ptr_UniformConstant__runtimearr_type_sampler UniformConstant
SamplerState      MySamplers[];
// CHECK: %MyCBuffers = OpVariable %_ptr_Uniform__runtimearr_type_ConstantBuffer_S Uniform
ConstantBuffer<S> MyCBuffers[];

float4 main(float2 loc : LOC) : SV_Target {
    return MyTextures[0].Sample(MySamplers[0], loc) + MyCBuffers[0].a;
}
