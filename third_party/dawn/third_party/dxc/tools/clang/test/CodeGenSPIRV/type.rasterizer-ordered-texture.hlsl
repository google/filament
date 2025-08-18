// RUN: %dxc -T ps_6_6 -E main -fcgl %s -spirv | FileCheck %s --check-prefixes=CHECK,INFER
// RUN: %dxc -fspv-use-unknown-image-format -T ps_6_6 -E main -fcgl %s -spirv | FileCheck %s --check-prefixes=CHECK,UNKNOWN

// CHECK: OpCapability Image1D

// INFER: %type_1d_image = OpTypeImage %int 1D 2 0 0 2 R32i
// UNKNOWN: %type_1d_image = OpTypeImage %int 1D 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_1d_image = OpTypePointer UniformConstant %type_1d_image
// INFER: %type_2d_image = OpTypeImage %uint 2D 2 0 0 2 Rg32ui
// UNKNOWN: %type_2d_image = OpTypeImage %uint 2D 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
// INFER: %type_3d_image = OpTypeImage %int 3D 2 0 0 2 R32i
// UNKNOWN: %type_3d_image = OpTypeImage %int 3D 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_3d_image = OpTypePointer UniformConstant %type_3d_image
// INFER: %type_3d_image_0 = OpTypeImage %float 3D 2 0 0 2 Rgba32f
// INFER: %_ptr_UniformConstant_type_3d_image_0 = OpTypePointer UniformConstant %type_3d_image_0
// INFER: %type_1d_image_array = OpTypeImage %int 1D 2 1 0 2 R32i
// INFER: %_ptr_UniformConstant_type_1d_image_array = OpTypePointer UniformConstant %type_1d_image_array
// INFER: %type_2d_image_array = OpTypeImage %uint 2D 2 1 0 2 Rg32ui
// INFER: %_ptr_UniformConstant_type_2d_image_array = OpTypePointer UniformConstant %type_2d_image_array
// INFER: %type_1d_image_array_0 = OpTypeImage %float 1D 2 1 0 2 Rgba32f
// INFER: %_ptr_UniformConstant_type_1d_image_array_0 = OpTypePointer UniformConstant %type_1d_image_array_0
// INFER: %type_2d_image_array_0 = OpTypeImage %float 2D 2 1 0 2 Rgba32f
// INFER: %_ptr_UniformConstant_type_2d_image_array_0 = OpTypePointer UniformConstant %type_2d_image_array_0


// CHECK: %t1 = OpVariable %_ptr_UniformConstant_type_1d_image UniformConstant
RasterizerOrderedTexture1D   <int>    t1 ;

// CHECK: %t2 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
RasterizerOrderedTexture2D   <uint2>  t2 ;

// CHECK: %t3 = OpVariable %_ptr_UniformConstant_type_3d_image UniformConstant
RasterizerOrderedTexture3D   <int>    t3 ;

// CHECK: %t4 = OpVariable %_ptr_UniformConstant_type_3d_image_0 UniformConstant
[[vk::image_format("rgba32f")]]
RasterizerOrderedTexture3D   <float3> t4 ;

// INFER: %t5 = OpVariable %_ptr_UniformConstant_type_3d_image_0 UniformConstant
// UNKNOWN: %t5 = OpVariable %_ptr_UniformConstant_type_3d_image_1 UniformConstant
RasterizerOrderedTexture3D   <float4> t5 ;

// CHECK: %t6 = OpVariable %_ptr_UniformConstant_type_1d_image_array UniformConstant
RasterizerOrderedTexture1DArray<int>    t6;

// CHECK: %t7 = OpVariable %_ptr_UniformConstant_type_2d_image_array UniformConstant
RasterizerOrderedTexture2DArray<uint2>  t7;

// CHECK: %t8 = OpVariable %_ptr_UniformConstant_type_1d_image_array_0 UniformConstant
RasterizerOrderedTexture1DArray<float4> t8;

// CHECK: %t9 = OpVariable %_ptr_UniformConstant_type_2d_image_array_0 UniformConstant
RasterizerOrderedTexture2DArray<float4> t9;

void main() {}
