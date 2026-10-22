// RUN: %dxc -Tlib_6_8 -verify %s

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024, 1, 1)]
[NodeIsProgramEntry]
void foo(uint3 tid : SV_GroupThreadId)
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024, 1, 1)]
[NodeIsProgramEntry]
void bar(uint3 tid : SV_GroupThreadID)
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024, 1, 1)]
[NodeIsProgramEntry]
void baz(uint3 tid : SV_VertexID) // expected-error {{Invalid system value semantic 'SV_VertexID' for launchtype 'Coalescing'}}
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024, 1, 1)]
[NodeIsProgramEntry]
void buz(uint tid : SV_GROUPINDEX)
{ }
