// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DVEC1 -DLHS_TY=uint16_t -DRHS_TY=uint %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DVEC1 -DLHS_TY=uint -DRHS_TY=uint16_t %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DVEC1 -DLHS_TY=bool -DRHS_TY=int %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DVEC1 -DLHS_TY=float -DRHS_TY=bool %s | FileCheck %s
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DVEC1 -DLHS_TY=float -DRHS_TY=float16_t %s | FileCheck %s


// This file tests cast between two constexpr-vectors having same component count, but different component types.

// CHECK: define void @main()


RWByteAddressBuffer rwbab;

void main() : OUT {
 const vector<LHS_TY, 1> v1 = vector<RHS_TY, 1>(0);
 rwbab.Store(256, v1);
 
 const vector<LHS_TY, 2> v2 = vector<RHS_TY, 2>(1, 2);
 rwbab.Store(512, v2);
 
 const vector<LHS_TY, 3> v3 = vector<RHS_TY, 3>(4, 5, 6);
 rwbab.Store(1024, v3);
 
 const vector<LHS_TY, 4> v4 = vector<RHS_TY, 4>(7, 8, 9, 10);
 rwbab.Store(2048, v4);
}