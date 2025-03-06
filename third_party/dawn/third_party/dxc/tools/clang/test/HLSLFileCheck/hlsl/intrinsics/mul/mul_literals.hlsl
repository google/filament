// This file contains tests covering all overloads of mul intrinsic
// as documented here: https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-mul

// RUN: %dxc -T vs_6_0 -E main %s  | FileCheck %s

struct DS {
  float f_1;
  float f_2;
  float f_3;
  float f_4;
  float2 f2_1;
  float2 f2_2;
  float3 f3_1;
  float3 f3_2;
  float4 f4_1;
  float4 f4_2;
  float2x3 fm_1;
  float3x4 fm_2;
  float2x4 fm_3;
  
  int i_1;
  int i_2;
  int i_3;
  int i_4;
  int2 i2_1;
  int2 i2_2;
  int3 i3_1;
  int3 i3_2;
  int4 i4_1;
  int4 i4_2;
  int2x3 im_1;
  int3x4 im_2;
  int2x4 im_3;
  
  uint u_1;
  uint u_2;
  uint u_3;
  uint u_4;
  uint2 u2_1;
  uint2 u2_2;
  uint3 u3_1;
  uint3 u3_2;
  uint4 u4_1;
  uint4 u4_2;
  uint2x3 um_1;
  uint3x4 um_2;
  uint2x4 um_3;
};

RWStructuredBuffer<DS> SB;

void main()
{
	//***************
	// float overloads
	//***************
	
    // scalar-scalar
	// CHECK: float 1.000000e+01
	SB[0].f_1 = mul(2, 5);

	// vector-vector
	// CHECK: float 1.100000e+01
	SB[0].f_2 = mul(float2(1, 2), float2(3, 4));
	
	// CHECK: float 2.300000e+01
	SB[0].f_3 = mul(float3(1, 2, 3), float3(3, 4, 4));
	
	// CHECK: float 3.200000e+01
	SB[0].f_4 = mul(float4(1, 2, 2, 1), float4(3, 4, 8, 5));
	
	// scalar-vector
	// CHECK: float 2.000000e+00, float 6.000000e+00
	SB[0].f2_1 = mul(2, float2(1, 3));
	
	// CHECK: float 3.000000e+00, float 9.000000e+00, float 0.000000e+00
	SB[0].f3_1 = mul(float3(1, 3, 0), 3);
	
	// CHECK: float 4.000000e+01, float 3.000000e+01, float 1.000000e+01, float 3.000000e+01
	SB[0].f4_1 = mul(10, float4(4, 3, 1, 3));	
	
	float2x3 m1 = {1, 2, 3, 4, 5, 6};
	float3x4 m2 = {7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
	float2x4 m3 = {2, 3, 4, 5, 6, 7, 8, 9};
	
	// scalar-matrix
	// CHECK: float 2.000000e+00, float 8.000000e+00, float 4.000000e+00, float 1.000000e+01
	// CHECK: float 6.000000e+00, float 1.200000e+01
	SB[0].fm_1 = mul(2, m1);
	
	// CHECK: float 1.330000e+02, float 2.090000e+02, float 2.850000e+02, float 1.520000e+02
	// CHECK: float 2.280000e+02, float 3.040000e+02, float 1.710000e+02, float 2.470000e+02
	// CHECK: float 3.230000e+02, float 1.900000e+02, float 2.660000e+02, float 3.420000e+02
	SB[0].fm_2 = mul(m2, 19);
	
	// matrix-matrix
	// CHECK: float 7.400000e+01, float 1.730000e+02, float 8.000000e+01, float 1.880000e+02
	// CHECK: float 8.600000e+01, float 2.030000e+02, float 9.200000e+01, float 2.180000e+02
	SB[0].fm_3 = mul(m1, m2);
	
	// vector-matrix
	// CHECK: float 1.400000e+01, float 3.200000e+01
	SB[0].f2_2 = mul(m1, float3(1, 2, 3));
	
	// CHECK: float 9.000000e+00, float 1.200000e+01, float 1.500000e+01
	SB[0].f3_2 = mul(float2(1, 2), m1);
	
	// CHECK: float 1.400000e+01, float 1.700000e+01, float 2.000000e+01, float 2.300000e+01
	SB[0].f4_2 = mul(float2(1, 2), m3);
	
	//***************
	// int overloads
	//***************
	
	// scalar-scalar
	// CHECK: i32 -10
	SB[0].i_1 = mul(2, -5);

	// vector-vector
	// CHECK: i32 -5
	SB[0].i_2 = mul(int2(1, -2), int2(3, 4));
	
	// CHECK: i32 7
	SB[0].i_3 = mul(int3(1, 2, 3), int3(3, -4, 4));
	
	// CHECK: i32 -16
	SB[0].i_4 = mul(int4(1, -2, 2, 1), int4(3, 4, -8, 5));
	
	// scalar-vector
	// CHECK: i32 2, i32 -6
	SB[0].i2_1 = mul(2, int2(1, -3));
	
	// CHECK: i32 3, i32 -9, i32 0
	SB[0].i3_1 = mul(int3(1, -3, 0), 3);
	
	// CHECK: i32 40, i32 30, i32 10, i32 30
	SB[0].i4_1 = mul(10, int4(4, 3, 1, 3));	
	
	int2x3 im1 = {1, 2, -3, 4, -5, 6};
	int3x4 im2 = {7, 8, 9, 10, -11, 12, 13, 14, -15, 16, -17, 18};
	int2x4 im3 = {2, 3, 4, -5, 6, 7, -8, -9};
	
	// scalar-matrix
	// CHECK: i32 -2, i32 -8, i32 -4, i32 10
	// CHECK: i32 6, i32 -12
	SB[0].im_1 = mul(-2, im1);
	
	// CHECK: i32 133, i32 -209, i32 -285, i32 152
	// CHECK: i32 228, i32 304, i32 171, i32 247
	// CHECK: i32 -323, i32 190, i32 266, i32 342
	SB[0].im_2 = mul(im2, 19);
	
	// matrix-matrix
	// CHECK: i32 30, i32 -7, i32 -16, i32 68
	// CHECK: i32 86, i32 -131, i32 -16, i32 78
	SB[0].im_3 = mul(im1, im2);
	
	// vector-matrix
	// CHECK: i32 -12, i32 32
	SB[0].i2_2 = mul(im1, int3(1, -2, 3));
	
	// CHECK: i32 9, i32 -8, i32 9
	SB[0].i3_2 = mul(int2(1, 2), im1);
	
	// CHECK: i32 10, i32 11, i32 -20, i32 -13
	SB[0].i4_2 = mul(int2(-1, 2), im3);
}