// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_2d_image = OpTypeImage %ulong 2D 2 0 0 2 R64ui
// CHECK: %_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image

// CHECK: %type_2d_image_0 = OpTypeImage %long 2D 2 0 0 2 R64i
// CHECK: %_ptr_UniformConstant_type_2d_image_0 = OpTypePointer UniformConstant %type_2d_image_0

// CHECK: %type_2d_image_1 = OpTypeImage %int 2D 2 0 0 2 Rgba16i
// CHECK: %_ptr_UniformConstant_type_2d_image_1 = OpTypePointer UniformConstant %type_2d_image_1

// CHECK: %tex_ui = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
RWTexture2D<uint64_t> tex_ui;

// CHECK: %tex_i = OpVariable %_ptr_UniformConstant_type_2d_image_0 UniformConstant
RWTexture2D<int64_t> tex_i;

// CHECK: %texout = OpVariable %_ptr_UniformConstant_type_2d_image_1 UniformConstant
RWTexture2D<min12int4> texout;

[numthreads(8, 8, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
    texout[id] =  tex_i[id] + tex_ui[id];
};
