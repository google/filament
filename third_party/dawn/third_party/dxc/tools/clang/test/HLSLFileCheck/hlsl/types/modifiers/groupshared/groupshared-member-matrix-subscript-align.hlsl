// RUN: %dxc -DSTYPE=float4x4 -DTYPE=Data /Tcs_6_0 %s | FileCheck %s -check-prefixes=CHECK,LCHECK,CHECK16,LCHECKF
// RUN: %dxc -DSTYPE=double4x4 -DTYPE=Data /Tcs_6_0 %s | FileCheck %s -check-prefixes=CHECK,LCHECK,CHECKD16,LCHECKF
// RUN: %dxc -DSTYPE=float16_t4x4 -DTYPE=Data /Tcs_6_2 %s -enable-16bit-types | FileCheck %s -check-prefixes=CHECK,LCHECK,CHECKH16,LCHECKH
// RUN: %dxc -DTYPE=float4x4 /Tcs_6_0 %s | FileCheck %s -check-prefixes=CHECK,LCHECK,CHECK4,LCHECKF
// RUN: %dxc -DTYPE=float4x1 /Tcs_6_0 %s | FileCheck %s -check-prefixes=CHECK,CHECK4
// RUN: %dxc -DTYPE=float1x4 /Tcs_6_0 %s | FileCheck %s -check-prefixes=CHECK,CHECK4
// RUN: %dxc -DTYPE=float2x2 /Tcs_6_0 %s | FileCheck %s -check-prefixes=CHECK,CHECK4
// RUN: %dxc -DTYPE=double4x4 /Tcs_6_0 %s | FileCheck %s -check-prefixes=CHECK,LCHECK,CHECK8,LCHECKF
// RUN: %dxc -DTYPE=float16_t4x4 /Tcs_6_2 %s -enable-16bit-types | FileCheck %s -check-prefixes=CHECK,LCHECK,CHECK2,LCHECKF

#ifndef STYPE
#define STYPE float4x4
#endif

struct Data {
   STYPE m;
};

// Ensure that the global has an alignment
//CHECK16: @"[[GV:\\01\?GData@@.*]]" = addrspace(3) global [[DIM:\[[0-9]+]] x [[ELTY:float]]] undef, align [[ALIGN:16]]
//CHECKD16: @"[[GV:\\01\?GData@@.*]]" = addrspace(3) global [[DIM:\[[0-9]+]] x [[ELTY:double]]] undef, align [[ALIGN:16]]
//CHECKH16: @"[[GV:\\01\?GData@@.*]]" = addrspace(3) global [[DIM:\[[0-9]+]] x [[ELTY:half]]] undef, align [[ALIGN:16]]
//CHECK8: @"[[GV:\\01\?GData@@.*]]" = addrspace(3) global [[DIM:\[[0-9]+]] x [[ELTY:double]]] undef, align [[ALIGN:8]]
//CHECK4: @"[[GV:\\01\?GData@@.*]]" = addrspace(3) global [[DIM:\[[0-9]+]] x [[ELTY:float]]] undef, align [[ALIGN:4]]
//CHECK2: @"[[GV:\\01\?GData@@.*]]" = addrspace(3) global [[DIM:\[[0-9]+]] x [[ELTY:half]]] undef, align [[ALIGN:2]]

groupshared TYPE GData;
StructuredBuffer<TYPE> input : register(t0);
RWStructuredBuffer<TYPE> output : register(u0);

[numthreads(128,1,1)]
void main(uint Id : SV_DispatchThreadId, uint g : SV_GroupID)
{
  // Ensure that the stores have the proper alignments
  // CHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 0), align [[ALIGN]]
  // CHECK16: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 1), align [[ALIGNIX1:4]]
  // CHECKD16: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 1), align [[ALIGNIX1:8]]
  // CHECKH16: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 1), align [[ALIGNIX1:2]]
  // CHECK8: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 1), align  [[ALIGNIX1:8]]
  // CHECK4: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 1), align  [[ALIGNIX1:4]]
  // CHECK2: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 1), align  [[ALIGNIX1:2]]
  // CHECK16: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 2), align [[ALIGNIX2:8]]
  // CHECKD16: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 2), align [[ALIGNIX2:16]]
  // CHECKH16: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 2), align [[ALIGNIX2:4]]
  // CHECK8: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 2), align [[ALIGNIX2:8]]
  // CHECK4: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 2), align [[ALIGNIX2:4]]
  // CHECK2: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 2), align [[ALIGNIX2:2]]
  // CHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 3), align [[ALIGNIX1]]
  // LCHECKF: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 4), align [[ALIGN]]
  // LCHECKH: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 4), align [[ALIGNIX4:8]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 5), align [[ALIGNIX1]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 6), align [[ALIGNIX2]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 7), align [[ALIGNIX1]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 8), align [[ALIGN]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 9), align [[ALIGNIX1]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 10), align [[ALIGNIX2]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 11), align [[ALIGNIX1]]
  // LCHECKF: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 12), align [[ALIGN]]
  // LCHECKH: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 12), align [[ALIGNIX4]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 13), align [[ALIGNIX1]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 14), align [[ALIGNIX2]]
  // LCHECK: store [[ELTY]] %{{[0-9]*}}, [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 15), align [[ALIGNIX1]]
  GData = input[0];
  GroupMemoryBarrierWithGroupSync();
  // Ensure that the loads have the proper alignments
  // CHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 0), align [[ALIGN]]
  // CHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 1), align [[ALIGNIX1]]
  // CHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 2), align [[ALIGNIX2]]
  // CHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 3), align [[ALIGNIX1]]
  // LCHECKF: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 4), align [[ALIGN]]
  // LCHECKH: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 4), align [[ALIGNIX4]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 5), align [[ALIGNIX1]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 6), align [[ALIGNIX2]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 7), align [[ALIGNIX1]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 8), align [[ALIGN]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 9), align [[ALIGNIX1]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 10), align [[ALIGNIX2]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 11), align [[ALIGNIX1]]
  // LCHECKF: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 12), align [[ALIGN]]
  // LCHECKH: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 12), align [[ALIGNIX4]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 13), align [[ALIGNIX1]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 14), align [[ALIGNIX2]]
  // LCHECK: load [[ELTY]], [[ELTY]] addrspace(3)* getelementptr inbounds ([[DIM]] x [[ELTY]]], [[DIM]] x [[ELTY]]] addrspace(3)* @"[[GV]]", i32 0, i32 15), align [[ALIGNIX1]]
  output[Id] = GData;

}
