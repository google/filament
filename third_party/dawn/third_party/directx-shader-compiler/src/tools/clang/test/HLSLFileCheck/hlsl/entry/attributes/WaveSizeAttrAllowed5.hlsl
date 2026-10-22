// RUN: %dxc -T lib_6_7 -ast-dump %s | FileCheck %s

// This test just checks whether compilation succeeds
// because we shouldn't presume that N() is an entry 
// point, so entry point attributes are ignored.

// CHECK: -HLSLWaveSizeAttr
// CHECK-SAME: 64

[WaveSize(64)]
void N(){}
