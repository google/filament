// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability Sampled1D

// CHECK: %type_1d_image = OpTypeImage %float 1D 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_1d_image = OpTypePointer UniformConstant %type_1d_image

// CHECK: %type_2d_image = OpTypeImage %int 2D 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image

// CHECK: %type_3d_image = OpTypeImage %uint 3D 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_3d_image = OpTypePointer UniformConstant %type_3d_image

// CHECK: %type_cube_image = OpTypeImage %float Cube 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_cube_image = OpTypePointer UniformConstant %type_cube_image

// CHECK: %type_1d_image_array = OpTypeImage %float 1D 2 1 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_1d_image_array = OpTypePointer UniformConstant %type_1d_image_array

// CHECK: %type_2d_image_array = OpTypeImage %int 2D 2 1 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_2d_image_array = OpTypePointer UniformConstant %type_2d_image_array

// CHECK: %type_cube_image_array = OpTypeImage %float Cube 2 1 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_cube_image_array = OpTypePointer UniformConstant %type_cube_image_array

// CHECK: %type_2d_image_0 = OpTypeImage %int 2D 2 0 1 1 Unknown
// CHECK: %_ptr_UniformConstant_type_2d_image_0 = OpTypePointer UniformConstant %type_2d_image_0

// CHECK: %type_2d_image_array_0 = OpTypeImage %uint 2D 2 1 1 1 Unknown
// CHECK: %_ptr_UniformConstant_type_2d_image_array_0 = OpTypePointer UniformConstant %type_2d_image_array_0

// CHECK: %t1 = OpVariable %_ptr_UniformConstant_type_1d_image UniformConstant
Texture1D   <float4> t1 : register(t1);
// CHECK: %t2 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
Texture2D   <int4>   t2 : register(t2);
// CHECK: %t3 = OpVariable %_ptr_UniformConstant_type_3d_image UniformConstant
Texture3D   <uint4>  t3 : register(t3);
// CHECK: %t4 = OpVariable %_ptr_UniformConstant_type_cube_image UniformConstant
TextureCube <float4> t4 : register(t4);

// CHECK: %t5 = OpVariable %_ptr_UniformConstant_type_1d_image_array UniformConstant
Texture1DArray   <float4> t5 : register(t5);
// CHECK: %t6 = OpVariable %_ptr_UniformConstant_type_2d_image_array UniformConstant
Texture2DArray   <int4>   t6 : register(t6);
// CHECK: %t7 = OpVariable %_ptr_UniformConstant_type_cube_image_array UniformConstant
TextureCubeArray <float4> t7 : register(t7);

// CHECK: %t8 = OpVariable %_ptr_UniformConstant_type_2d_image_0 UniformConstant
Texture2DMS      <int3>   t8 : register(t8);
// CHECK: %t9 = OpVariable %_ptr_UniformConstant_type_2d_image_array_0 UniformConstant
Texture2DMSArray <uint4>  t9 : register(t9);

// CHECK: %t10 = OpVariable %_ptr_UniformConstant_type_2d_image_1 UniformConstant
Texture2D   <bool>   t10 : register(t10);

void main() {
// CHECK-LABEL: %main = OpFunction
}
