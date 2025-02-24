// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: local resource not guaranteed to map to unique global resource

RWStructuredBuffer<float2x2> oA;
RWStructuredBuffer<float2x2> oB;
RWStructuredBuffer<float2x2> oC;

StructuredBuffer<float2x2> iA;
StructuredBuffer<float2x2> iB;

uint s;

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
RWStructuredBuffer<float2x2> o = s>7? oA: oB;
StructuredBuffer<float2x2> input = s>7? iA : iB;

o = s>9? o : oC;

for (uint i=0;i<4;i++) {
  input  =  i%2==0 ? input : iB;
o[gid.x] = input [gid.x];
}

}
