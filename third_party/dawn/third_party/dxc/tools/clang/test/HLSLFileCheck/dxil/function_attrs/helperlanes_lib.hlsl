// RUN: %dxc -T lib_6_7 %s | FileCheck %s
// RUN: %dxc -T lib_6_6 %s | FileCheck -check-prefix=IGNORED %s

// IGNORED-NOT: waveops-include-helper-lanes

[WaveOpsIncludeHelperLanes]
[shader("compute")]
[numthreads(1,1,1)]
void cs_main() {
}


[WaveOpsIncludeHelperLanes]
[shader("pixel")]
float ps_main(int N : A, int C : B) : SV_TARGET {
    return (float)N;
}

// CHECK: define void @cs_main() #[[CSMainAttr:[0-9]+]]
// CHECK: define void @ps_main() #[[PSMainAttr:[0-9]+]]

// CHECK-NOT: attribtes #[[CSMainAttr]] = {{.*}}"waveops-include-helper-lanes"{{.*}}
// CHECK: attributes #[[PSMainAttr]] = {{.*}}"waveops-include-helper-lanes"{{.*}}