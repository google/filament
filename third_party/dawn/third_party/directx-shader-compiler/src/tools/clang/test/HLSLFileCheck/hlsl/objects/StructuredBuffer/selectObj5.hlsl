// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: local resource not guaranteed to map to unique global resource
// CHECK: local resource not guaranteed to map to unique global resource

RWStructuredBuffer<float2x2> o1;
RWStructuredBuffer<float2x2> o2;

StructuredBuffer<float2x2> i1;
StructuredBuffer<float2x2> i2;

uint s;

void run(uint id) {
RWStructuredBuffer<float2x2> ot = s > 8 ? o2:o1;
StructuredBuffer<float2x2> it = s < 7 ? i1:i2;
ot[id] = it[id];
}

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
   run(tid.x);
}
