// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %MyBuffer DescriptorSet 0
// CHECK: OpDecorate %MyBuffer Binding 0
// CHECK: OpDecorate %MyRWBuffer DescriptorSet 0
// CHECK: OpDecorate %MyRWBuffer Binding 1
// CHECK: OpDecorate %MyTexture DescriptorSet 0
// CHECK: OpDecorate %MyTexture Binding 2
// CHECK: OpDecorate %MyRWTexture DescriptorSet 0
// CHECK: OpDecorate %MyRWTexture Binding 3
// CHECK: OpDecorate %MySamplers DescriptorSet 0
// CHECK: OpDecorate %MySamplers Binding 4
// CHECK: OpDecorate %MyCompSamplers DescriptorSet 0
// CHECK: OpDecorate %MyCompSamplers Binding 5

// CHECK: %type_buffer_image = OpTypeImage %float Buffer 2 0 0 1 Rgba32f
// CHECK: %_arr_type_buffer_image_uint_1 = OpTypeArray %type_buffer_image %uint_1

// CHECK: %type_buffer_image_0 = OpTypeImage %float Buffer 2 0 0 2 Rgba32f
// CHECK: %_arr_type_buffer_image_0_uint_2 = OpTypeArray %type_buffer_image_0 %uint_2

// CHECK: %type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
// CHECK: %_arr_type_2d_image_uint_3 = OpTypeArray %type_2d_image %uint_3

// CHECK: %type_2d_image_0 = OpTypeImage %float 2D 2 0 0 2 Rgba32f
// CHECK: %_arr_type_2d_image_0_uint_4 = OpTypeArray %type_2d_image_0 %uint_4

// CHECK: %type_sampler = OpTypeSampler
// CHECK: %_arr_type_sampler_uint_5 = OpTypeArray %type_sampler %uint_5
// CHECK: %_arr_type_sampler_uint_6 = OpTypeArray %type_sampler %uint_6

// CHECK:       %MyBuffer = OpVariable %_ptr_UniformConstant__arr_type_buffer_image_uint_1 UniformConstant
Buffer<float4>         MyBuffer[1];
// CHECK:     %MyRWBuffer = OpVariable %_ptr_UniformConstant__arr_type_buffer_image_0_uint_2 UniformConstant
RWBuffer<float4>       MyRWBuffer[2];
// CHECK:      %MyTexture = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_3 UniformConstant
Texture2D<float4>      MyTexture[3];
// CHECK:    %MyRWTexture = OpVariable %_ptr_UniformConstant__arr_type_2d_image_0_uint_4 UniformConstant
RWTexture2D<float4>    MyRWTexture[4];
// CHECK:     %MySamplers = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_5 UniformConstant
SamplerState           MySamplers[5];
// CHECK: %MyCompSamplers = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_6 UniformConstant
SamplerComparisonState MyCompSamplers[6];

// TODO: unsized arrays of resources

float4 main() : SV_Target {
// CHECK:   [[MyBuffer:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_buffer_image %MyBuffer %int_0
// CHECK:            {{%[0-9]+}} = OpLoad %type_buffer_image [[MyBuffer]]
    return MyBuffer[0].Load(1) +
// CHECK: [[MyRWBuffer:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_buffer_image_0 %MyRWBuffer %int_1
// CHECK:            {{%[0-9]+}} = OpLoad %type_buffer_image_0 [[MyRWBuffer]]
           MyRWBuffer[1][2] +
// CHECK:  [[MyTexture:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %MyTexture %int_2
// CHECK:            {{%[0-9]+}} = OpLoad %type_2d_image [[MyTexture]]
// CHECK:  [[MySampler:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %MySamplers %int_3
// CHECK:            {{%[0-9]+}} = OpLoad %type_sampler [[MySampler]]
           MyTexture[2].Sample(MySamplers[3], float2(0.1, 0.2));
}
