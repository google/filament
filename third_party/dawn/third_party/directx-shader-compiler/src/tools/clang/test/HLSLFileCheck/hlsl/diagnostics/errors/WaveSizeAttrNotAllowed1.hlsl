// RUN: %dxc -E S -T cs_6_0 %s | FileCheck %s

// CHECK: error: attribute wavesize requires shader model 6.6 or greater

[WaveSize(64)]
[numthreads(2,2,4)]
void S()
{
    return;
}
