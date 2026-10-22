// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure select on resource index.
// CHECK: select i1 {{.*}}, i32 2, i32 1
// CHECK: select i1 {{.*}}, i32 0, i32 3


RWStructuredBuffer<float2x2> o[6];

StructuredBuffer<float2x2> i[6];

uint s;

void run(uint id) {
RWStructuredBuffer<float2x2> ot = s > 8 ? o[2]:o[1];
StructuredBuffer<float2x2> it = s < 7 ? i[0]:i[3];
ot[id] = it[id];
}

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
   run(tid.x);
}
