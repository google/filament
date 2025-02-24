// RUN: %dxc -E S -T cs_6_7 %s | FileCheck %s

// CHECK: error: shader attribute type 'wavesize' conflicts with shader attribute type 'wavesize'
// CHECK: note: conflicting attribute is here

[WaveSize(64)]
[WaveSize(32)]
[numthreads(2,2,4)]
void S()
{
    return;
}
