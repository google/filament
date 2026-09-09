// RUN: %dxc -E main -T cs_6_0 -Od %s | FileCheck %s

// CHECK: @main

RWStructuredBuffer<float2x2> oA[6];

StructuredBuffer<float2x2> iA[6];

uint s;

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{

RWStructuredBuffer<float2x2> o = oA[0];
StructuredBuffer<float2x2> i = iA[0];

if (s > 8) {
    for (uint a = 0;a<3;a++) {
      o = oA[a];
      i = iA[a];
      o[gid.x+a] = i[gid.x+a];
    }

}

o[gid.x] = i[gid.x];

}
