// RUN: %dxc -T lib_6_8 %s -verify

// expected-error@+7{{Invalid system value semantic 'SV_DispatchThreadID' for launchtype 'Thread'}}
// expected-error@+6{{Invalid system value semantic 'SV_GroupID' for launchtype 'Thread'}}
// expected-error@+5{{Invalid system value semantic 'SV_GroupThreadID' for launchtype 'Thread'}}
// expected-error@+4{{Invalid system value semantic 'SV_GroupIndex' for launchtype 'Thread'}}
[shader("node")]
[NodeLaunch("thread")]
[numthreads(1,1,1)]
void entry( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
}