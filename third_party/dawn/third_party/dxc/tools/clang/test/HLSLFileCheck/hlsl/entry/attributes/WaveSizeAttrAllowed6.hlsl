// RUN: %dxc -T lib_6_7 -ast-dump %s | FileCheck %s

// CHECK: -HLSLWaveSizeAttr
// CHECK-SAME: 64

[WaveSize(64)]
[numthreads(2,2,4)]
void S()
{
    return;
}
