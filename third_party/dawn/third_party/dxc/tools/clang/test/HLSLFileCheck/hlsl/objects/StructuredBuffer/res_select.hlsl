// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure resource select inside loop works.
// CHECK: bufferLoad

RWStructuredBuffer<float2x2> oA;
RWStructuredBuffer<float2x2> oB;

StructuredBuffer<float2x2> iA;
StructuredBuffer<float2x2> iB;

uint s;

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
[unroll]
for (uint i=0;i<4;i++) {
  RWStructuredBuffer<float2x2> o = oA;
  StructuredBuffer<float2x2> ibuf = iA;

  if (i > 2) {
    o = oB;
    ibuf = iB;
  }
  o[gid.x] = ibuf[gid.x];

}

}