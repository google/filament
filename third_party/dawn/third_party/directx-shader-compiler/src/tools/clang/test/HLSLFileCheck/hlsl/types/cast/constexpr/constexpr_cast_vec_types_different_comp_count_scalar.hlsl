// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DLHS_TY=uint -DRHS_TY=uint %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DLHS_TY=float16_t -DRHS_TY=float16_t %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DLHS_TY=bool -DRHS_TY=float16_t %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DLHS_TY=uint16_t -DRHS_TY=float %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DLHS_TY=uint -DRHS_TY=bool %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DLHS_TY=bool -DRHS_TY=uint %s | FileCheck %s

// This file tests truncation cast between two constexpr-vectors with different component count and/or types.

// CHECK: define void @main()


RWByteAddressBuffer rwbab;

void main() : OUT {

 // Case 1: all zero constant
 const LHS_TY s0 = vector<RHS_TY, 4>(0, 0, 0, 0);
 rwbab.Store(100, s0);
 
 const vector<LHS_TY, 1> v1 = vector<RHS_TY, 4>(0, 0, 0, 0);
 rwbab.Store(200, v1);
 
 const vector<LHS_TY, 2> v2 = vector<RHS_TY, 4>(0, 0, 0, 0);
 rwbab.Store(300, v2);
 
 const vector<LHS_TY, 3> v3 = vector<RHS_TY, 4>(0, 0, 0, 0);
 rwbab.Store(400, v3);
 
 const vector<LHS_TY, 4> v4 = vector<RHS_TY, 4>(0, 0, 0, 0);
 rwbab.Store(500, v4);
 
 // Case 2: Non-zero constant
 const LHS_TY s1 = vector<RHS_TY, 4>(1, 2, 3, 4);
 rwbab.Store(600, s1);
 
 const vector<LHS_TY, 1> v5 = vector<RHS_TY, 4>(1, 2, 3, 4);
 rwbab.Store(700, v5);
 
 const vector<LHS_TY, 2> v6 = vector<RHS_TY, 4>(1, 2, 3, 4);
 rwbab.Store(800, v6);
 
 const vector<LHS_TY, 3> v7 = vector<RHS_TY, 4>(1, 2, 3, 4);
 rwbab.Store(900, v7);
 
 const vector<LHS_TY, 4> v8 = vector<RHS_TY, 4>(1, 2, 3, 4);
 rwbab.Store(1000, v8);

}