// RUN: %dxc -Tlib_6_8 -verify %s
// ==================================================================
// Invalid NodeLaunch value
// This failure prevents some other diagnostics being produced so
// this test can't be combined with others
// ==================================================================

[Shader("node")]
[NodeLaunch("Other")]  // expected-error {{attribute 'NodeLaunch' must have one of these values: broadcasting,coalescing,thread}}
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node01()
{ }



