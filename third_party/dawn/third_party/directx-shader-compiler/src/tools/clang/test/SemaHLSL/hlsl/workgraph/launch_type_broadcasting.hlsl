// RUN: %dxc -T lib_6_8 %s -verify

// expected-no-diagnostics
[shader("node")]
[NodeDispatchGrid(1,1,1)]
[NodeLaunch("broadcasting")]
[numthreads(1,1,1)]
void entry2( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
}