// RUN: %dxc /T cs_6_0 /E cs_1 -Vd %s -verify
// This test verifies that no warnings are emitted
// when an entry point attribute exists on a function
// without a shader attribute

// expected-no-diagnostics

[numthreads(64,1,1)]
void cs_1() {}

[numthreads(64,1,1)]
void cs_2() {}
