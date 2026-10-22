// RUN: %dxc -T lib_6_8 %s -verify

// expected-error@+5{{Invalid system value semantic 'SV_DispatchThreadID' for launchtype 'Coalescing'}}
// expected-error@+4{{Invalid system value semantic 'SV_GroupID' for launchtype 'Coalescing'}}
[shader("node")]
[NodeLaunch("coalescing")]
[numthreads(1,1,1)]
void entry3( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
}