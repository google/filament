// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: ; Buffer Definitions:

// CHECK: ; Resource bind info for mats
// CHECK: ; {

// CHECK: ;   struct hostlayout.struct.mat
// CHECK: ;   {

// CHECK: ;       row_major float2x2 f2x2;                      ; Offset:    0

// CHECK: ;   } $Element;                                       ; Offset:    0 Size:    16

// CHECK: ; }

// CHECK: ; Resource bind info for mats2
// CHECK: ; {

// CHECK: ;   column_major float2x2 $Element;                                ; Offset:    0 Size:    16

// CHECK: ; }

// CHECK: ; Resource bind info for fA
// CHECK: ; {

// CHECK: ;   column_major float2x2 $Element;                                ; Offset:    0 Size:    16

// CHECK: ; }

// CHECK: ; Resource Bindings:

// CHECK: ; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
// CHECK: ; ------------------------------ ---------- ------- ----------- ------- -------------- ------
// CHECK: ; mats                              texture  struct         r/o      T0             t0     1
// CHECK: ; mats2                             texture  struct         r/o      T1             t1     1
// CHECK: ; fA                                    UAV  struct         r/w      U0             u0     1

// CHECK: threadId
// CHECK: groupId
// CHECK: threadIdInGroup
// CHECK: flattenedThreadIdInGroup
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: i32 8
// CHECK: barrier
// CHECK: i32 9
// CHECK: barrier
// CHECK: i32 10
// CHECK: barrier
// CHECK: i32 11
// CHECK: barrier
// CHECK: i32 2
// CHECK: barrier
// CHECK: i32 4

groupshared column_major float2x2 dataC[8*8];

RWStructuredBuffer<float2x2> fA;

struct mat {
  row_major float2x2 f2x2;
};

StructuredBuffer<mat> mats;
StructuredBuffer<float2x2> mats2;

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
    dataC[tid.x%(8*8)] = mats.Load(gid.x).f2x2 + mats2.Load(gtid.y);
	GroupMemoryBarrier();
    GroupMemoryBarrierWithGroupSync();
    float2x2 f2x2 = dataC[8*8-1-tid.y%(8*8)];
  AllMemoryBarrier();
       fA[gidx+2] = f2x2; 
  AllMemoryBarrierWithGroupSync();
      fA[gidx+1] = f2x2;
  DeviceMemoryBarrier();
  DeviceMemoryBarrierWithGroupSync();
    fA[gidx] = f2x2;
}
