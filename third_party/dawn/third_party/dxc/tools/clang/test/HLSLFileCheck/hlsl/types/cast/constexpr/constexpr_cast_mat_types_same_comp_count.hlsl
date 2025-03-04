// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types %s | FileCheck %s

// This test checks that constexpr cast between matrices of same dimension but different component type compile fines.

// CHECK: define void @main()


void main() : OUT {
 const float1x1 v1 = min16float1x1(0); 
 const int1x1 v2 = min16float1x1(-1);
 
 const uint1x2 v3 = int1x2(0, 0);
 const double1x2 v4 = bool1x2(1, 2);
 
 const bool2x1 v5 = float2x1(0, 0);
 const min16int2x1 v6 = double2x1(1, 2);
 
 const uint2x2 v7 = int2x2(0, 0, 0, 0);
 const int2x2 v8 = min16uint2x2(1, 2, 3, 4);

 const uint2x3 v9 = double2x3(0, 0, 0, 0, 0, 0);
 const min16int2x3 v10 = min16float2x3(1, 2, 3, 4, 5, 6);
 
 const uint3x2 v11 = double3x2(0, 0, 0, 0, 0, 0);
 const min16int3x2 v12 = min16float3x2(1, 2, 3, 4, 5, 6);
 
 const bool3x3 v13 = min16float3x3(0, 0, 0, 0, 0, 0, 0, 0, 0);
 const uint3x3 v14 = min16int3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
 
 const bool4x4 v15 = min16float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
 const int4x4 v16 = double4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
}