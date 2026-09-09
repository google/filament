// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_2d_image = OpTypeImage %float 2D 2 0 0 2 Rgba16f
// CHECK: %_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image

// CHECK: %type_2d_image_0 = OpTypeImage %uint 2D 2 0 0 2 Rgba16ui
// CHECK: %_ptr_UniformConstant_type_2d_image_0 = OpTypePointer UniformConstant %type_2d_image_0

// CHECK: %type_2d_image_1 = OpTypeImage %int 2D 2 0 0 2 Rgba16i
// CHECK: %_ptr_UniformConstant_type_2d_image_1 = OpTypePointer UniformConstant %type_2d_image_1

// CHECK:    %tex = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
RWTexture2D<min10float4> tex;

// CHECK: %tex_ui = OpVariable %_ptr_UniformConstant_type_2d_image_0 UniformConstant
RWTexture2D<min16uint4> tex_ui;

// CHECK: %texout = OpVariable %_ptr_UniformConstant_type_2d_image_1 UniformConstant
RWTexture2D<min12int4> texout;

[numthreads(8, 8, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
    texout[id] =  tex[id] + tex_ui[id];
};
