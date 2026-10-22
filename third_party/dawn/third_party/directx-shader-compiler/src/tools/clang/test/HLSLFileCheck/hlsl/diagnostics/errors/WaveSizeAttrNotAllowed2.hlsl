// RUN: %dxc -T cs_6_6 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_0 -E main %s | FileCheck %s

// this test should pass, because the WaveSize 
// attribute should be ignored if it's not on a 
// known entry point function for this compilation,
// even if the target profile is < 6.6

// CHECK: @main, !"main", null, null, [[PROPS:![0-9]+]]}
// CHECK: [[PROPS]] = !{i32 4, [[NT:![0-9]+]]}
// CHECK: [[NT]] = !{i32 2, i32 2, i32 1}

[wavesize(32)]
[numthreads(2,2,1)]
void main2() {}

[numthreads(2,2,1)]
void main() {main2();}
