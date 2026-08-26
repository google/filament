// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types %s | FileCheck %s

// This test checks that constexpr truncation cast involving matrix type.

// CHECK: define void @main()


void main() : OUT {
 const float v1 = min16float1x1(0); 
 const int v2 = min16float1x1(-1);
 
 const uint v3 = int1x2(0, 0);
 const double v4 = bool1x2(1, 2);
 
 const bool v5 = float2x1(0, 0);
 const min16int v6 = double2x1(1, 2);
 
 const uint v7 = int2x2(0, 0, 0, 0);
 const int v8 = min16uint2x2(1, 2, 3, 4);

 const uint v9 = double2x3(0, 0, 0, 0, 0, 0);
 const min16int v10 = min16float2x3(1, 2, 3, 4, 5, 6);
 
 const uint v11 = double3x2(0, 0, 0, 0, 0, 0);
 const min16int v12 = min16float3x2(1, 2, 3, 4, 5, 6);
 
 const bool v13 = min16float3x3(0, 0, 0, 0, 0, 0, 0, 0, 0);
 const uint v14 = min16int3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
 
 const bool v15 = min16float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
 const int v16 = double4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
 
 const uint1x1 v17 = int1x2(0, 0);
 const double1x1 v18 = bool1x2(1, 2);
 
 const bool1x1 v19 = float2x1(0, 0);
 const min16int1x1 v20 = double2x1(1, 2);
 
 const uint2x1 v21 = int2x2(0, 0, 0, 0);
 const int2x1 v22 = min16uint2x2(1, 2, 3, 4);
 
 const uint1x2 v23 = int2x2(0, 0, 0, 0);
 const int1x2 v24 = min16uint2x2(1, 2, 3, 4);

 const uint2x2 v25 = double2x3(0, 0, 0, 0, 0, 0);
 const min16int2x1 v26 = min16float2x3(1, 2, 3, 4, 5, 6);
 
 const uint1x2 v27 = double3x2(0, 0, 0, 0, 0, 0);
 const min16int1x1 v28 = min16float3x2(1, 2, 3, 4, 5, 6);
 
 const bool2x3 v29 = min16float3x3(0, 0, 0, 0, 0, 0, 0, 0, 0);
 const uint3x2 v30 = min16int3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
 
 const bool1x1 v31 = min16float3x3(0, 0, 0, 0, 0, 0, 0, 0, 0);
 const uint2x2 v32 = min16int3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
 
 const bool1x1 v33 = min16float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
 const int2x3 v34 = double4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
 
 const bool3x3 v35 = min16float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
 const int3x4 v36 = double4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
}