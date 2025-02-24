// RUN: %dxc -E main -T cs_6_0 -Od %s | FileCheck %s


// CHECK: @dx.op.cbufferLoadLegacy.f32(i32 59
// CHECK: 3
// CHECK: extractvalue
// CHECK: 2
// CHECK: extractvalue
// CHECK: 3

RWStructuredBuffer<float> o;

float4x4 m;

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
o[gid.x] = m._34 + m._44;
}
