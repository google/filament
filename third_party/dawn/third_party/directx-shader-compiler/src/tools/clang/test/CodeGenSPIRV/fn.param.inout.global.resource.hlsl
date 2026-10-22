// RUN: %dxc -E main -T ps_6_0 -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

Texture2D<float4>               r0;
RWTexture3D<float4>             r1;
SamplerState                    r2;
RaytracingAccelerationStructure r3;
RWBuffer<float4>                r4;
ByteAddressBuffer               r5;
RWByteAddressBuffer             r6;
RWStructuredBuffer<float4>      r7;
AppendStructuredBuffer<float4>  r8;

float4 run(inout    Texture2D<float4>               a0,
           inout    RWTexture3D<float4>             a1,
           inout    SamplerState                    a2,
           inout    RaytracingAccelerationStructure a3,
           inout    RWBuffer<float4>                a4,
           inout    ByteAddressBuffer               a5,
           inout    RWByteAddressBuffer             a6,
           inout    RWStructuredBuffer<float4>      a7,
           inout    AppendStructuredBuffer<float4>  a8)
{
    float4 pos = a4.Load(0);
    return a0.Sample(a2, float2(a6.Load(pos.x), a5.Load(pos.y)));
}

float4 main(): SV_Target
{
// CHECK: %param_var_a0 = OpVariable %_ptr_Function_type_2d_image Function
// CHECK: %param_var_a1 = OpVariable %_ptr_Function_type_3d_image Function
// CHECK: %param_var_a2 = OpVariable %_ptr_Function_type_sampler Function
// CHECK: %param_var_a3 = OpVariable %_ptr_Function_accelerationStructureNV Function
// CHECK: %param_var_a4 = OpVariable %_ptr_Function_type_buffer_image Function
// CHECK: %param_var_a5 = OpVariable %_ptr_Function__ptr_StorageBuffer_type_ByteAddressBuffer Function
// CHECK: %param_var_a6 = OpVariable %_ptr_Function__ptr_StorageBuffer_type_RWByteAddressBuffer Function
// CHECK: %param_var_a7 = OpVariable %_ptr_Function__ptr_StorageBuffer_type_RWStructuredBuffer_v4float Function
// CHECK: %param_var_a8 = OpVariable %_ptr_Function__ptr_StorageBuffer_type_AppendStructuredBuffer_v4float Function

// CHECK: [[r0:%[a-zA-Z0-9_]+]] = OpLoad %type_2d_image %r0
// CHECK:               OpStore %param_var_a0 [[r0]]
// CHECK: [[r1:%[a-zA-Z0-9_]+]] = OpLoad %type_3d_image %r1
// CHECK:               OpStore %param_var_a1 [[r1]]
// CHECK: [[r2:%[a-zA-Z0-9_]+]] = OpLoad %type_sampler %r2
// CHECK:               OpStore %param_var_a2 [[r2]]
// CHECK: [[r3:%[a-zA-Z0-9_]+]] = OpLoad %accelerationStructureNV %r3
// CHECK:               OpStore %param_var_a3 [[r3]]
// CHECK: [[r4:%[a-zA-Z0-9_]+]] = OpLoad %type_buffer_image %r4
// CHECK:               OpStore %param_var_a4 [[r4]]
// CHECK:               OpStore %param_var_a5 %r5
// CHECK:               OpStore %param_var_a6 %r6
// CHECK:               OpStore %param_var_a7 %r7
// CHECK:               OpStore %param_var_a8 %r8

// CHECK: OpFunctionCall %v4float %run %param_var_a0 %param_var_a1 %param_var_a2 %param_var_a3 %param_var_a4 %param_var_a5 %param_var_a6 %param_var_a7 %param_var_a8

    return run(r0, r1, r2, r3, r4, r5, r6, r7, r8);
}
