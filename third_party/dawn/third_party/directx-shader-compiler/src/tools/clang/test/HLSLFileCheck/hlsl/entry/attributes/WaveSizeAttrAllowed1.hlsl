// RUN: %dxc -T lib_6_6 -ast-dump %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -ast-dump %s | FileCheck %s

// This test used to fail because it didn't take into account
// the possibility of the compute shader being a library target.
// Now, this test is an example of what should be allowed
// and no errors are expected.

// CHECK: -HLSLWaveSizeAttr
// CHECK-SAME: 32 0 0

[Shader("compute")]
[WaveSize(32)]
[numthreads(2,2,1)]
void CS()
{
    return;
}