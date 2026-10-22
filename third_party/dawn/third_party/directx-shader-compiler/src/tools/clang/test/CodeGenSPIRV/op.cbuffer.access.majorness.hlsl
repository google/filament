// RUN: %dxc -T cs_6_0 -E main -Zpr -fcgl  %s -spirv | FileCheck %s

// CHECK: %SData = OpTypeStruct %_arr_mat3v4float_uint_2 %_arr_mat3v4float_uint_2_0
struct SData {
                float3x4 mat1[2];
   column_major float3x4 mat2[2];
};

// CHECK: %type_SBufferData = OpTypeStruct %SData %_arr_mat3v4float_uint_2 %_arr_mat3v4float_uint_2_0 %mat3v4float
cbuffer SBufferData {
                SData    BufferData;
                float3x4 Mat1[2];
   column_major float3x4 Mat2[2];
   row_major    float3x4 Mat3;
};

// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_SData %SBufferData %int_0
// CHECK: [[val:%[0-9]+]] = OpLoad %SData [[ptr]]
// CHECK:     {{%[0-9]+}} = OpCompositeExtract %_arr_mat3v4float_uint_2 [[val]] 0
// CHECK:     {{%[0-9]+}} = OpCompositeExtract %_arr_mat3v4float_uint_2_0 [[val]] 1
static const SData Data = BufferData;

// CHECK: [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_SData %SBufferData %int_0
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_Uniform__arr_mat3v4float_uint_2 [[ptr_0]] %int_0
static const float3x4 Matrices[2] = BufferData.mat1;

// CHECK: [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_SData %SBufferData %int_0
// CHECK:  [[base:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_mat3v4float_uint_2_0 [[ptr_1]] %int_1
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_mat3v4float [[base]] %int_1
static const float3x4 Matrix = BufferData.mat2[1];

RWStructuredBuffer<float4> Out;

[numthreads(4, 4, 4)]
void main() {
// CHECK: [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_mat3v4float_uint_2 %SBufferData %int_1
// CHECK:     {{%[0-9]+}} = OpLoad %_arr_mat3v4float_uint_2 [[ptr_2]]
  float3x4 a[2] = Mat1;
// CHECK: [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_mat3v4float_uint_2_0 %SBufferData %int_2
// CHECK:     {{%[0-9]+}} = OpLoad %_arr_mat3v4float_uint_2_0 [[ptr_3]]
  float3x4 b[2] = Mat2;

// CHECK: [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_mat3v4float_uint_2_0 %SBufferData %int_2
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_mat3v4float [[ptr_4]] %int_1
  float3x4 c = Mat2[1];
// CHECK:     {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_mat3v4float %SBufferData %int_3
  float3x4 d = Mat3;

  Out[0] = Data.mat1[0][0];
}
